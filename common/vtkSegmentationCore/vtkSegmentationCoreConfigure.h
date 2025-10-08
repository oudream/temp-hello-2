#ifndef VTKSEGMENTATIONCORECONFIGURE_H
#define VTKSEGMENTATIONCORECONFIGURE_H

#define vtkSegmentationCore_STATIC

#if defined(WIN32) && !defined(vtkSegmentationCore_STATIC)
#pragma warning ( disable : 4275 )

#if defined(vtkSegmentationCore_EXPORTS)
#define vtkSegmentationCore_EXPORT __declspec( dllexport )
#else
#define vtkSegmentationCore_EXPORT __declspec( dllimport )
#endif
#else
#define vtkSegmentationCore_EXPORT
#endif

#endif