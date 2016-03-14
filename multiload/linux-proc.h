#ifndef LINUX_PROC_H__
#define LINUX_PROC_H__

#include <load-graph.h>

G_GNUC_INTERNAL void GetCpu (int Maximum, int data [5], LoadGraph *g);
G_GNUC_INTERNAL void GetDisk (int Maximum, int data [3], LoadGraph *g);
G_GNUC_INTERNAL void GetTemperature (int Maximum, int data [2], LoadGraph *g);
G_GNUC_INTERNAL void GetMemory (int Maximum, int data [4], LoadGraph *g);
G_GNUC_INTERNAL void GetSwap (int Maximum, int data [2], LoadGraph *g);
G_GNUC_INTERNAL void GetLoadAvg (int Maximum, int data [2], LoadGraph *g);
G_GNUC_INTERNAL void GetNet (int Maximum, int data [4], LoadGraph *g);

#endif
