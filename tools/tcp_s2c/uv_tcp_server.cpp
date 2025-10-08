#include "uv_tcp_server.h"

#include <queue>
#include "uv_tcp_client.h"


#include <ccxx/cxtime.h>


using namespace std;


#define DEFAULT_BACKLOG 0x200
#define IO_BUFFER_SIZE 0x1000
#define MODULE_NAME "S2C-SERVER"
#define MAX_TASK_COUNT (1024*4)

static uv_loop_t *f_loop;
static uv_tcp_t f_tcpServer;
static uv_timer_t f_timerPrint;

class RecvThread;

struct RecvTask {
    RecvTask() :client(NULL),thread(NULL),time(0),index(0),increment(0),total(0)
    {
        memset(buffer,0,IO_BUFFER_SIZE);
    }
    ~RecvTask() {}
    uv_stream_t *client;
    RecvThread *thread;
    msepoch_t time;
    size_t index;
    size_t increment;
    size_t total;
    uint8_t buffer[IO_BUFFER_SIZE];
    string ip;

};

struct RecvBuffer {
    string buffer;
    RecvTask *task;
};



// *** *** ***   *** *** ***   *** *** ***   *** *** ***   *** *** ***   *** *** ***   *** *** ***   *** *** ***   *** *** ***   *** *** ***   *** *** ***   *** *** ***
// --- --- ---   --- --- ---   --- --- ---   --- --- ---   --- --- ---   --- --- ---   --- --- ---   --- --- ---   --- --- ---   --- --- ---   --- --- ---   --- --- ---
// log
// --- --- ---   --- --- ---   --- --- ---   --- --- ---   --- --- ---   --- --- ---   --- --- ---   --- --- ---   --- --- ---   --- --- ---   --- --- ---   --- --- ---
// *** *** ***   *** *** ***   *** *** ***   *** *** ***   *** *** ***   *** *** ***   *** *** ***   *** *** ***   *** *** ***   *** *** ***   *** *** ***   *** *** ***

static string logPath;
static FILE *logFd = NULL;
static int logSize = 0;
#define ME_GOAHEAD_LOGMAXSIZE (1024*1024*1024-1)

static int logOpen();
static void logClose();

static void logCheck()
{
    if (logFd == NULL)
    {
        logOpen();
    }
    else if (logSize > ME_GOAHEAD_LOGMAXSIZE)
    {
        logClose();
        logOpen();
    }
}

static void logDefaultHandler(const char *buf)
{
    logCheck();
    int bufLen = (int) strlen(buf);
    if (logFd != NULL)
    {
        std::string dt = CxTime::currentMsepochString() + " ";
//        printf("%s", dt.c_str());
//        printf("%s", buf);
//        printf("%s", "\n");
        fwrite(dt.c_str(), 1, dt.size(), logFd);
        fwrite(buf, 1, bufLen, logFd);
        fwrite("\n", 1, 1, logFd);
        fflush(logFd);
        logSize += bufLen + 1 + dt.size();
    }
}

static int logOpen()
{
    if (logPath.empty())
    {
        logPath = CxFileSystem::getCurrentExePath()+".server.log";
    }
    logFd = fopen(logPath.c_str(), "w");
    if (logFd == NULL)
    {
        return -1;
    }
    fseek(logFd, 0, SEEK_SET);
    logSize = 0;
    return 0;
}

static void logClose()
{
    if (logFd != NULL)
    {
        fclose(logFd);
        logFd = NULL;
    }
}

static void logPrint(const char *format, va_list args)
{
    char buffer[4096];
    vsnprintf(buffer, sizeof(buffer), format, args);
    logDefaultHandler(buffer);
}

static void logInfo(const char *format, ...)
{
    std::string fmt = std::string("INFO -> ") + std::string(format);
    va_list args;
    va_start(args, format);
    logPrint(fmt.c_str(), args);
    va_end(args);
}

static void logDebug(const char *format, ...)
{
    std::string fmt = std::string("DEBUG > ") + std::string(format);
    va_list args;
    va_start(args, format);
    logPrint(fmt.c_str(), args);
    va_end(args);
}

static void logWarn(const char *format, ...)
{
    std::string fmt = std::string("WARN -- ") + std::string(format);
    va_list args;
    va_start(args, format);
    logPrint(fmt.c_str(), args);
    va_end(args);
}

static void logError(const char *format, ...)
{
    std::string fmt = std::string("ERROR - ") + std::string(format);
    va_list args;
    va_start(args, format);
    logPrint(fmt.c_str(), args);
    va_end(args);
}






// *** *** ***   *** *** ***   *** *** ***   *** *** ***   *** *** ***   *** *** ***   *** *** ***   *** *** ***   *** *** ***   *** *** ***   *** *** ***   *** *** ***
// --- --- ---   --- --- ---   --- --- ---   --- --- ---   --- --- ---   --- --- ---   --- --- ---   --- --- ---   --- --- ---   --- --- ---   --- --- ---   --- --- ---
// recv
// --- --- ---   --- --- ---   --- --- ---   --- --- ---   --- --- ---   --- --- ---   --- --- ---   --- --- ---   --- --- ---   --- --- ---   --- --- ---   --- --- ---
// *** *** ***   *** *** ***   *** *** ***   *** *** ***   *** *** ***   *** *** ***   *** *** ***   *** *** ***   *** *** ***   *** *** ***   *** *** ***   *** *** ***


static void fetchFrame(const unsigned char *buf, int len, RecvTask *task)
{
#define PACKET_HEAD 0x7B7B
#define PACKET_TAIL 0x7D7D
    //
    // cout << task->index << "len: " << len << ", client: " << task->client << ", time: " << task->time << endl;
    if (len > IO_BUFFER_SIZE)
    {
        return;
    }
    size_t index = task->index;
    unsigned char *pBegin = &task->buffer[index];
    memcpy(pBegin, buf, len);
    unsigned char *pEnd = pBegin + len -1;
    unsigned char *p = pBegin;
    unsigned char *pHead = index > 0 ? task->buffer : NULL;
    unsigned char *pTail = NULL;
    while (p < pEnd)
    {
        if (pHead != NULL)
        {
            if (*((unsigned short *)p) == PACKET_TAIL)
            {
                pTail = p;
                // send
                UVTcpClient::sendData(pHead, pTail - pHead + 2);
//                cout << CxString::toHexstring(pHead, pTail - pHead + 2) << endl;
                pHead = NULL;
                pTail = NULL;
            }
        }
        else
        {
            if (*((unsigned short *)p) == PACKET_HEAD)
            {
                // send
                pHead = p;
                pTail = NULL;
            }
        }
        p++;
    }
    if (pHead != NULL)
    {
        size_t n = (pBegin + len)-pHead;
        if (n < IO_BUFFER_SIZE)
        {
            memcpy(&task->buffer[0], pHead, n);
            task->index = n;
        }
        else
        {
            task->index = 0;
        }
    }
    else
    {
        task->index = 0;
    }
}

class RecvThread: public CxJoinableThread
{
public:
    RecvThread()
    {
        _pushBuffers = &_buffers1;
        _popBuffers = &_buffers2;
        _lock = LX_PTHREAD_MUTEX_INITIALIZER;
    }

    void push(const char *buf, int len, RecvTask *task)
    {
        (void)cx_pthread_mutex_lock(&_lock);
        {
            RecvBuffer recvBuffer;
            recvBuffer.buffer = string(buf, len);
            recvBuffer.task = task;
            _pushBuffers->push(recvBuffer);
        }
        (void)cx_pthread_mutex_unlock(&_lock);
        _singleWait.signal();
    }

    void waitExit()
    {
        _run = false;
        _singleWait.signal();
        join();
    }

    size_t count;

protected:
    void run(void) override
    {
        _run = true;
        while (_run)
        {
            _singleWait.wait();

            (void)cx_pthread_mutex_lock(&_lock);
            queue<RecvBuffer> * tmpBuffer = _pushBuffers;
            _pushBuffers = _popBuffers;
            _popBuffers = tmpBuffer;
            (void)cx_pthread_mutex_unlock(&_lock);

            while (!_popBuffers->empty()) {
                RecvBuffer &buffer = _popBuffers->front();
                string &buf = buffer.buffer;
                RecvTask *task = buffer.task;
//                UVTcpClient::sendData((const unsigned char *)buffer.buffer.c_str(), buffer.buffer.size());
                fetchFrame(reinterpret_cast<const unsigned char *>(buf.c_str()), buf.size(), task);
                _popBuffers->pop();
            }
        }
    }

private:
    CxSingleWait _singleWait;
    queue<RecvBuffer> _buffers1;
    queue<RecvBuffer> _buffers2;
    queue<RecvBuffer> *_pushBuffers;
    queue<RecvBuffer> *_popBuffers;
    cx_pthread_mutex_t _lock;
    volatile bool _run;

};

#define RECV_THREAD_NUM 4
static RecvThread f_recvThreads[RECV_THREAD_NUM];
static map<uv_stream_t*, RecvTask> f_recvTasks;
static uv_async_t f_sendAsync;
static cx_pthread_mutex_t f_sendTaskQueueMutex = LX_PTHREAD_MUTEX_INITIALIZER;

struct SendTask {
    SendTask():size(0){
        memset(buffer,0,IO_BUFFER_SIZE);
    }
    ~SendTask() {}
    uint8_t buffer[IO_BUFFER_SIZE];
    size_t  size;
};

static queue<struct SendTask> f_sendTaskQueue;

typedef struct {
    uv_write_t req;
    uv_buf_t buf;
} WriteReq_t;


static RecvThread * allocateRecvThread()
{
    RecvThread *min = &f_recvThreads[0];
    size_t count = SIZE_MAX;
    for (int i = 0; i < RECV_THREAD_NUM; ++i)
    {
        RecvThread *thread = &f_recvThreads[i];
        if (thread->count < count)
        {
            min = thread;
            count = min->count;
        }
    }
    min->count ++;
    return min;
}

static void startRecvThreads()
{
    for (int i = 0; i < RECV_THREAD_NUM; ++i)
    {
        RecvThread *thread = &f_recvThreads[i];
        thread->start();
    }
}

static void stopRecvThreads()
{
    for (int i = 0; i < RECV_THREAD_NUM; ++i)
    {
        RecvThread *thread = &f_recvThreads[i];
        thread->waitExit();
    }
}

static void cleanRecvThread()
{
    msepoch_t dtMax = f_recvTasks.size() > 10000 ? 3 * 1000 : (f_recvTasks.size() > 1000 ? 1 * 60 * 1000 : 60 * 60 * 1000);
    msepoch_t dtNow = CxTime::currentMsepoch();
    vector<uv_stream_t*> ds;
    for(map<uv_stream_t*,RecvTask>::iterator it = f_recvTasks.begin(); it != f_recvTasks.end(); ++it)
    {
        RecvTask &task = it->second;
        if (task.time - dtNow > dtMax)
        {
            ds.push_back(it->first);
            task.thread->count--;
        }
    }
    CxContainer::remove(f_recvTasks, ds);
}

static void on_close(uv_handle_t* handle) {
    free(handle);
}

static void alloc_buffer(uv_handle_t *handle, size_t suggested_size, uv_buf_t *buf) {
    *buf = uv_buf_init((char*)malloc(4097), 4097);
}

static void echo_read(uv_stream_t* client, ssize_t nread, const uv_buf_t *buf)
{
    if (nread > 0)
    {
        map<uv_stream_t*, RecvTask>::iterator it1 = f_recvTasks.find(client);
        if (it1 != f_recvTasks.end())
        {
            RecvTask &task = it1->second;
            task.increment += nread;
            task.thread->push(buf->base, nread,&task);
        }
        else
        {
            if (f_recvTasks.size() > MAX_TASK_COUNT)
            {
                cleanRecvThread();
            }
            RecvThread *thread = allocateRecvThread();
            RecvTask t;
            t.client = client;
            t.thread = thread;
            // get ip dress
            uv_os_fd_t fd;
            int r = uv_fileno(reinterpret_cast<const uv_handle_t *>(client), &fd);
            if (r >= 0)
            {
                t.ip = CxSocket::getPeerAddress(socket_t(fd));
            }
            //
            f_recvTasks[client] = t;
            map<uv_stream_t*, RecvTask>::iterator it2 = f_recvTasks.find(client);
            if (it2 != f_recvTasks.end())
            {
                RecvTask &task = it2->second;
                task.increment += nread;
                task.thread->push(buf->base, nread,&task);
            }
            else
            {
                logError(MODULE_NAME".%s %d Inter error f_recvTasks[client] can not find.", __func__ , __LINE__);
            }
        }
    }

    if (nread < 0)
    {
        if (nread != UV_EOF)
        {
            logError(MODULE_NAME".%s %d Read error %s.", __func__ , __LINE__, uv_err_name(nread));
        }
        uv_close((uv_handle_t*)client, on_close);
    }

    free(buf->base);
}


static void on_new_connection(uv_stream_t* server, int status) {
    if (status < 0) {
        logError(MODULE_NAME".%s %d New connection error %s.", __func__ , __LINE__, uv_strerror(status));
        return ;
    }

    uv_tcp_t *client = (uv_tcp_t*) malloc(sizeof(uv_tcp_t));
    uv_tcp_init(f_loop, client);
    if (uv_accept(server, (uv_stream_t*)client) == 0) {
        uv_read_start((uv_stream_t*)client, alloc_buffer, echo_read);
    } else {
        uv_close((uv_handle_t*)client, on_close);
    }
}


// *** *** ***   *** *** ***   *** *** ***   *** *** ***   *** *** ***   *** *** ***   *** *** ***   *** *** ***   *** *** ***   *** *** ***   *** *** ***   *** *** ***
// --- --- ---   --- --- ---   --- --- ---   --- --- ---   --- --- ---   --- --- ---   --- --- ---   --- --- ---   --- --- ---   --- --- ---   --- --- ---   --- --- ---
// send
// --- --- ---   --- --- ---   --- --- ---   --- --- ---   --- --- ---   --- --- ---   --- --- ---   --- --- ---   --- --- ---   --- --- ---   --- --- ---   --- --- ---
// *** *** ***   *** *** ***   *** *** ***   *** *** ***   *** *** ***   *** *** ***   *** *** ***   *** *** ***   *** *** ***   *** *** ***   *** *** ***   *** *** ***

static void free_write_req(uv_write_t *req) {
    WriteReq_t *wr = (WriteReq_t*) req;
    free(wr->buf.base);
    free(wr);
}

static void echo_write(uv_write_t *req, int status) {
    if (status) {
        logError(MODULE_NAME".%s %d Write error %s.", __func__ , __LINE__, uv_strerror(status));
    }
    free_write_req(req);
}

static void send_walk_cb(uv_handle_t* handle, void* arg) {
    uv_handle_type type = uv_handle_get_type(handle);
    if (type == UV_TCP && uv_is_active(handle))
    {
        if (handle != (uv_handle_t*)&f_tcpServer)
        {
            SendTask *tsk = (SendTask*)arg;

            WriteReq_t *req = (WriteReq_t*)malloc(sizeof(WriteReq_t));
            req->buf = uv_buf_init((char*)malloc(tsk->size),tsk->size);
            memcpy(req->buf.base, tsk->buffer, tsk->size);
            uv_write((uv_write_t*)req, (uv_stream_t*)handle, &req->buf, 1, echo_write);
        }
    }
}

static void send_task_run(uv_async_t *handle)
{
    (void)cx_pthread_mutex_lock(&f_sendTaskQueueMutex);
    while (!f_sendTaskQueue.empty()) {
        struct SendTask &tsk = f_sendTaskQueue.front();
        uv_walk(f_loop, send_walk_cb, &tsk);
        logInfo(MODULE_NAME".%s %d Send %d.", __func__ , __LINE__, int(tsk.size));
        f_sendTaskQueue.pop();
    }
    (void)cx_pthread_mutex_unlock(&f_sendTaskQueueMutex);
}


// *** *** ***   *** *** ***   *** *** ***   *** *** ***   *** *** ***   *** *** ***   *** *** ***   *** *** ***   *** *** ***   *** *** ***   *** *** ***   *** *** ***
// --- --- ---   --- --- ---   --- --- ---   --- --- ---   --- --- ---   --- --- ---   --- --- ---   --- --- ---   --- --- ---   --- --- ---   --- --- ---   --- --- ---
// timer print
// --- --- ---   --- --- ---   --- --- ---   --- --- ---   --- --- ---   --- --- ---   --- --- ---   --- --- ---   --- --- ---   --- --- ---   --- --- ---   --- --- ---
// *** *** ***   *** *** ***   *** *** ***   *** *** ***   *** *** ***   *** *** ***   *** *** ***   *** *** ***   *** *** ***   *** *** ***   *** *** ***   *** *** ***

static void timer_timeout_print(uv_timer_t *handle)
{
    stringstream ss;
    if (f_recvTasks.size() < MAX_TASK_COUNT)
    {
        ss << MODULE_NAME".RecvTasks.Report.Begin(" << f_recvTasks.size() << "):\n";
        int i = 0;
        long long total = 0;
        for(std::map<uv_stream_t*, RecvTask>::iterator it = f_recvTasks.begin(); it != f_recvTasks.end(); ++it)
        {
            RecvTask &task = it->second;
            task.total += task.increment;
            ss << i++ << "(" << task.ip << "," << task.time << "<-" << task.total << "++" << task.increment << "),\n";
            task.increment = 0;
            total += task.total;
        }
        ss << MODULE_NAME".RecvTasks.Report.End {Count: " << f_recvTasks.size() << ", Total: " << total << " }.\n";
    }
    else
    {
        ss << "WARN -- " << MODULE_NAME".RecvTasks.Count Tool More ( " << f_recvTasks.size() << " )\n\n";
    }
    string s = ss.str();
    logDefaultHandler(s.c_str());
}

int UVTcpServer::start(int iPort)
{
    f_loop = uv_loop_new();
    uv_tcp_init(f_loop, &f_tcpServer);

    struct sockaddr_in addr;
    uv_ip4_addr("0.0.0.0", iPort, &addr);
    uv_tcp_bind(&f_tcpServer, (const struct sockaddr*)&addr, 0);

    int r = uv_listen((uv_stream_t*)&f_tcpServer, DEFAULT_BACKLOG, on_new_connection);
    if (r) {
        logError(MODULE_NAME".%s %d Listen error %s.", __func__ , __LINE__, uv_strerror(r));
        return -1;
    }

    // start recv ( work ) thread
    startRecvThreads();

    // init async init
    uv_async_init(f_loop, &f_sendAsync, send_task_run);

    // timer
    uv_timer_init(f_loop, &f_timerPrint);
    uv_timer_start(&f_timerPrint, timer_timeout_print, 0, 5000);

    // while(true)
    return uv_run(f_loop, UV_RUN_DEFAULT);
}

void UVTcpServer::stop()
{
    // stop timer
    uv_timer_stop(&f_timerPrint);
    uv_close((uv_handle_t*)&f_timerPrint, NULL);
    // stop loop
    uv_stop(f_loop);
    // stop recv ( work ) thread
    stopRecvThreads();
    // Wake up the event loop
    uv_async_send(&f_sendAsync);
}

int UVTcpServer::pushData(const unsigned char *pData, int iLength)
{
    struct SendTask new_task;
    new_task.size = iLength;
    memcpy(new_task.buffer, pData, new_task.size);

    (void)cx_pthread_mutex_lock(&f_sendTaskQueueMutex);
    f_sendTaskQueue.push(new_task);
    (void)cx_pthread_mutex_unlock(&f_sendTaskQueueMutex);

    return uv_async_send(&f_sendAsync);
}
