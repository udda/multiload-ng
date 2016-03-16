#ifndef __LINUX_PROC_H__
#define __LINUX_PROC_H__

#include "autoscaler.h"
#include "load-graph.h"


G_BEGIN_DECLS

#define NCPUSTATES 5
#define PER_CPU_MAX_LOADAVG 3

typedef struct _CpuData {
	guint32 cpu_time [NCPUSTATES];
	guint32 cpu_last [NCPUSTATES];

	gfloat user;
	gfloat iowait;
	gfloat total_use;
	guint64 num_cpu;
} CpuData;

typedef struct _MemoryData {
	guint64 user;
	guint64 cache;
	guint64 total;
} MemoryData;

typedef struct _NetData {
	NetSpeed *in;
	NetSpeed *out;
} NetData;

typedef struct _SwapData {
	guint64 used;
	guint64 total;
} SwapData;

typedef struct _LoadAvgData {
	double loadavg[3];
} LoadAvgData;

typedef struct _DiskData {
	guint64 last_read;
	guint64 last_write;
	AutoScaler scaler;

	guint64 read_speed;
	guint64 write_speed;
} DiskData;

typedef struct _TemperatureData {
	guint16 temperature;
} TemperatureData;


G_GNUC_INTERNAL void
GetCpu (int Maximum, int data [5], LoadGraph *g);
G_GNUC_INTERNAL void
GetMemory (int Maximum, int data [4], LoadGraph *g);
G_GNUC_INTERNAL void
GetNet (int Maximum, int data [4], LoadGraph *g);
G_GNUC_INTERNAL void
GetSwap (int Maximum, int data [2], LoadGraph *g);
G_GNUC_INTERNAL void
GetLoadAvg (int Maximum, int data [2], LoadGraph *g);
G_GNUC_INTERNAL void
GetDisk (int Maximum, int data [3], LoadGraph *g);
G_GNUC_INTERNAL void
GetTemperature (int Maximum, int data [2], LoadGraph *g);

G_END_DECLS

#endif /* __LINUX_PROC_H__ */
