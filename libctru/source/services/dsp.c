#include <3ds/types.h>
#include <3ds/svc.h>
#include <3ds/srv.h>
#include <3ds/ipc.h>
#include <3ds/services/dsp.h>

Handle dspHandle = 0;


Result dspInit(void)
{
	Result ret = 0;

	if (dspHandle == 0)
	{
		ret = srvGetServiceHandle(&dspHandle, "dsp::DSP");
		if (ret < 0) return ret;
	}
	if (ret < 0) return ret;
	DSP_UnloadComponent();
	return 0;
}

Result dspExit(void)
{
	Result ret = 0;
//No need to call unload, it will be done automatically by closing the handle
	if (dspHandle != 0)
	{
		ret = svcCloseHandle(dspHandle);
		if (ret < 0) return ret;
		dspHandle = 0;
	}

	return 0;
}


Result DSP_GetHeadphoneStatus(bool* is_inserted)
{
	Result ret = 0;
	u32* cmdbuf = getThreadCommandBuffer();
	cmdbuf[0] = IPC_MakeHeader(0x001F,0,0);
	if ((ret = svcSendSyncRequest(dspHandle)) != 0) return ret;
	*is_inserted = cmdbuf[2] & 0xFF;
	return cmdbuf[1];
}


Result DSP_FlushDataCache(u32 address, u32 size)
{
	Result ret = 0;
	u32* cmdbuf = getThreadCommandBuffer();
	cmdbuf[0] = IPC_MakeHeader(0x13,2,2);
	cmdbuf[1] = address;
	cmdbuf[2] = size;
	cmdbuf[3] = IPC_Desc_SharedHandles(1);
	cmdbuf[4] = CUR_PROCESS_HANDLE;
	if ((ret = svcSendSyncRequest(dspHandle)) != 0) return ret;
	return cmdbuf[1];
}


Result DSP_InvalidateDataCache(u32 address, u32 size)
{
	Result ret = 0;
	u32* cmdbuf = getThreadCommandBuffer();
	cmdbuf[0] = IPC_MakeHeader(0x14,2,2);
	cmdbuf[1] = address;
	cmdbuf[2] = size;
	cmdbuf[3] = IPC_Desc_SharedHandles(1);
	cmdbuf[4] = CUR_PROCESS_HANDLE;
	if ((ret = svcSendSyncRequest(dspHandle)) != 0) return ret;
	return cmdbuf[1];
}



Result DSP_SetSemaphore(u16 value)
{
	Result ret = 0;
	u32* cmdbuf = getThreadCommandBuffer();
	cmdbuf[0] = IPC_MakeHeader(0x7,1,0);
	cmdbuf[1] = value;
	if ((ret = svcSendSyncRequest(dspHandle)) != 0) return ret;
	return cmdbuf[1];
}



Result DSP_SetSemaphoreMask(u16 mask)
{
	Result ret = 0;
	u32* cmdbuf = getThreadCommandBuffer();
	cmdbuf[0] = IPC_MakeHeader(0x17,1,0);
	cmdbuf[1] = mask;
	if ((ret = svcSendSyncRequest(dspHandle)) != 0) return ret;
	return cmdbuf[1];
}

Result DSP_GetSemaphoreHandle(Handle* semaphore)
{
	Result ret = 0;
	u32* cmdbuf = getThreadCommandBuffer();
	cmdbuf[0] = IPC_MakeHeader(0x16,0,0);
	if ((ret = svcSendSyncRequest(dspHandle)) != 0) return ret;
	*semaphore = cmdbuf[3];
	return cmdbuf[1];
}

Result DSP_LoadComponent(u8 const* component,u32 size,u16 prog_mask,u16 data_mask,bool * is_loaded)
{
	Result ret = 0;
	u32* cmdbuf = getThreadCommandBuffer();
	cmdbuf[0] = IPC_MakeHeader(0x11,3,2);
	cmdbuf[1] = size;
	cmdbuf[2] = prog_mask;
	cmdbuf[3] = data_mask;
	cmdbuf[4] = IPC_Desc_Buffer(size,IPC_BUFFER_R);
	cmdbuf[5] = (u32) component;
	if ((ret = svcSendSyncRequest(dspHandle)) != 0) return ret;
	*is_loaded = cmdbuf[2] & 0xFF;
	return cmdbuf[1];
}



Result DSP_UnloadComponent(void)
{
	Result ret = 0;
	u32* cmdbuf = getThreadCommandBuffer();
	cmdbuf[0] = IPC_MakeHeader(0x12,0,0);
	if ((ret = svcSendSyncRequest(dspHandle)) != 0) return ret;
	return cmdbuf[1];
}

Result DSP_RegisterInterruptEvents(Handle handle, u32 interrupt, u32 channel)
{
	Result ret = 0;
	u32* cmdbuf = getThreadCommandBuffer();
	cmdbuf[0] = IPC_MakeHeader(0x15,2,2);
	cmdbuf[1] = interrupt;
	cmdbuf[2] = channel;
	cmdbuf[3] = IPC_Desc_SharedHandles(1);
	cmdbuf[4] = handle;
	if ((ret = svcSendSyncRequest(dspHandle)) != 0) return ret;
	return cmdbuf[1];
}


Result DSP_ReadPipeIfPossibleEx(u32 channel,u32 unk1, u8 const *buffer, u16 length, u16* length_read)
{
	Result ret = 0;
	u32* cmdbuf = getThreadCommandBuffer();
	cmdbuf[0] = IPC_MakeHeader(0x10,3,0);
	cmdbuf[1] = channel;
	cmdbuf[2] = unk1;
	cmdbuf[3] = length;

	u32 * staticbufs = cmdbuf + 0x100;

	u32 saved1 = staticbufs[0x0];
	u32 saved2 = staticbufs[0x4];

	staticbufs[0] = (length<<14) | 2;
	staticbufs[4] = (u32)buffer;

	if ((ret = svcSendSyncRequest(dspHandle)) != 0) return ret;

	staticbufs[0] = saved1;
	staticbufs[4] = saved2;

	*length_read = cmdbuf[2] & 0xFFFF;
	return cmdbuf[1];
}

//TODO change DSP_ReadPipeIfPossibleEx into DSP_ReadPipeIfPossible once unk1 is figured out
//However it seems that it is always used with value 0
Result DSP_ReadPipeIfPossible(u32 channel, u8 const *buffer, u16 length, u16* length_read)
{
	return DSP_ReadPipeIfPossibleEx(channel,0,buffer,length, length_read);
}

Result DSP_WriteProcessPipe(u32 channel, u8 const *buffer, u32 length)
{
	Result ret = 0;
	u32* cmdbuf = getThreadCommandBuffer();
	cmdbuf[0] = IPC_MakeHeader(0xd,2,2);
	cmdbuf[1] = channel;
	cmdbuf[2] = length;
	cmdbuf[3] = IPC_Desc_StaticBuffer(length,1);
	cmdbuf[4] = (u32) buffer;
	if ((ret = svcSendSyncRequest(dspHandle)) != 0) return ret;
	return cmdbuf[1];
}

Result DSP_ConvertProcessAddressFromDspDram(u32 dsp_address, u32 *arm_address)
{
	Result ret = 0;
	u32* cmdbuf = getThreadCommandBuffer();
	cmdbuf[0] = IPC_MakeHeader(0xc,1,0);
	cmdbuf[1] = dsp_address;
	if ((ret = svcSendSyncRequest(dspHandle)) != 0) return ret;
	*arm_address = cmdbuf[2];
	return cmdbuf[1];
}

Result DSP_RecvData(u16 regNo, u16 * value)
{
	Result ret = 0;
	u32* cmdbuf = getThreadCommandBuffer();
	cmdbuf[0] = IPC_MakeHeader(0x1,1,0) ;
	cmdbuf[1] = regNo;
	if ((ret = svcSendSyncRequest(dspHandle)) != 0) return ret;
	*value = cmdbuf[2] & 0xFFFF;
	return cmdbuf[1];
}

Result DSP_RecvDataIsReady(u16 regNo, bool * is_ready)
{
	Result ret = 0;
	u32* cmdbuf = getThreadCommandBuffer();
	cmdbuf[0] = IPC_MakeHeader(0x2,1,0);
	cmdbuf[1] = regNo;
	if ((ret = svcSendSyncRequest(dspHandle)) != 0) return ret;
	*is_ready = cmdbuf[2] & 0xFF;
	return cmdbuf[1];
}


// Writes data to the reg regNo
// *(_WORD *)(8 * regNo + 0x1ED03024) = value
Result DSP_SendData(u16 regNo, u16 value)
{
	Result ret = 0;
	u32* cmdbuf = getThreadCommandBuffer();
	cmdbuf[0] = IPC_MakeHeader(0x3,2,0);
	cmdbuf[1] = regNo;
	cmdbuf[1] = value;
	if ((ret = svcSendSyncRequest(dspHandle)) != 0) return ret;
	return cmdbuf[1];
}

Result DSP_SendDataIsEmpty(u16 regNo, bool * is_empty)
{
	Result ret = 0;
	u32* cmdbuf = getThreadCommandBuffer();
	cmdbuf[0] = IPC_MakeHeader(0x4,1,0);
	cmdbuf[1] = regNo;
	if ((ret = svcSendSyncRequest(dspHandle)) != 0) return ret;
	*is_empty = cmdbuf[2] & 0xFF;
	return cmdbuf[1];
}
