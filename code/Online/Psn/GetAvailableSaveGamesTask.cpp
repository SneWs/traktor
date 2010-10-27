#include <sysutil/sysutil_gamecontent.h>
#include "Core/Io/DynamicMemoryStream.h"
#include "Core/Log/Log.h"
#include "Core/Misc/AutoPtr.h"
#include "Core/Serialization/BinarySerializer.h"
#include "Online/Psn/GetAvailableSaveGamesTask.h"
#include "Online/Psn/SaveGamePsn.h"
#include "Online/Psn/LogError.h"

namespace traktor
{
	namespace online
	{
		namespace
		{

const uint32_t c_maxDirCount = 32;
const uint32_t c_maxFileCount = 32;

char s_indicatorDispMsg[CELL_SAVEDATA_INDICATORMSG_MAX] = "Loading...";
const char s_secureFileId[CELL_SAVEDATA_SECUREFILEID_SIZE] = {
	'C', 'L', 'E', 'A', 
	'R', 'H', 'E', 'A', 
	'D', 'G', 'A', 'M', 
	'E', 'S', ' ', ' ', 
};

CellSaveDataAutoIndicator s_indicator = 
{
	dispPosition : CELL_SAVEDATA_INDICATORPOS_LOWER_RIGHT | CELL_SAVEDATA_INDICATORPOS_MSGALIGN_RIGHT,
	dispMode : CELL_SAVEDATA_INDICATORMODE_BLINK,
	dispMsg : s_indicatorDispMsg,
	picBufSize : 0,
	picBuf : NULL,
	reserved : NULL
};

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.online.GetAvailableSaveGamesTask", GetAvailableSaveGamesTask, ISaveGameQueueTask)

Ref< ISaveGameQueueTask > GetAvailableSaveGamesTask::create(RefArray< ISaveGame >& outSaveGames, int32_t requiredTrophySizeKB)
{
	Ref< GetAvailableSaveGamesTask > task = new GetAvailableSaveGamesTask();
	task->m_outSaveGames = &outSaveGames;
	task->m_requiredTrophySizeKB = requiredTrophySizeKB;
	return task;
}

bool GetAvailableSaveGamesTask::execute()
{
	uint32_t tmpSize = std::max< uint32_t >(
		c_maxDirCount * sizeof(CellSaveDataDirList),
		c_maxFileCount * sizeof(CellSaveDataFileStat)
	);
	AutoArrayPtr< uint8_t > tmp(new uint8_t [tmpSize]);

	CellSaveDataSetList list;
	std::memset(&list, 0, sizeof(list));
	list.sortType = CELL_SAVEDATA_SORTTYPE_MODIFIEDTIME;
	list.sortOrder = CELL_SAVEDATA_SORTORDER_DESCENT;
	list.dirNamePrefix = "NPEB00401-PUZZLED-";

	CellSaveDataSetBuf buf;
	std::memset(&buf, 0, sizeof(buf));
	buf.dirListMax = c_maxDirCount;
	buf.fileListMax = c_maxFileCount;
	buf.bufSize = tmpSize;
	buf.buf = tmp.ptr();

	int32_t err = cellSaveDataListAutoLoad(
		CELL_SAVEDATA_VERSION_CURRENT,
		CELL_SAVEDATA_ERRDIALOG_NOREPEAT,
		&list,
		&buf,
		&callbackLoadFixed,
		&callbackLoadStat,
		&callbackLoadFile,
		SYS_MEMORY_CONTAINER_ID_INVALID,
		(void*)this
	);

	bool oldSaveDataExists = !this->m_loadBuffer.empty();
	int32_t neededGameDataSizeKB = oldSaveDataExists ? 4 : 100;

	const int32_t NEED_SIZEKB = this->m_hddFreeSpaceKB - this->m_requiredTrophySizeKB - neededGameDataSizeKB;
	if (NEED_SIZEKB < 0)
	{
		cellGameContentErrorDialog(CELL_GAME_ERRDIALOG_NOSPACE_EXIT, -NEED_SIZEKB, NULL); 
		log::error << L"Unable to create session manager. Not enough space on HDD to save trophies and savegames" << Endl;
		return false;
	}
	if (!oldSaveDataExists)
		return false;

	if (err != CELL_SAVEDATA_RET_OK)
	{
		LogError::logErrorSaveData(err);
		// EXIT APP
		return false;
	}

	return true;
}

void GetAvailableSaveGamesTask::callbackLoadFixed(CellSaveDataCBResult* cbResult, CellSaveDataListGet* get, CellSaveDataFixedSet* set)
{
	set->dirName = "NPEB00401-PUZZLED-0001";
	set->newIcon = 0;
	set->option = CELL_SAVEDATA_OPTION_NONE;
	cbResult->result = CELL_SAVEDATA_CBRESULT_OK_NEXT;
}

void GetAvailableSaveGamesTask::callbackLoadStat(CellSaveDataCBResult* cbResult, CellSaveDataStatGet* get, CellSaveDataStatSet* set)
{
	GetAvailableSaveGamesTask* this_ = static_cast< GetAvailableSaveGamesTask* >(cbResult->userdata);
	T_ASSERT (this_);

	this_->m_hddFreeSpaceKB = get->hddFreeSizeKB;
	if (get->isNewData)
	{
		cbResult->result = CELL_SAVEDATA_CBRESULT_OK_LAST;
		return;
	}

	if (get->fileNum > get->fileListNum)
	{
		cbResult->result = CELL_SAVEDATA_CBRESULT_ERR_BROKEN;
		return;
	}

	int32_t i = 0;
	for (; i < get->fileListNum; ++i)
	{
		if (strcmp(get->fileList[i].fileName, "ATT.BIN") == 0)
		{
			this_->m_loadBuffer.resize(get->fileList[i].st_size);
			this_->m_loadBufferPending = true;
			break;
		}
	}
	if (i >= get->fileListNum)
	{
		cbResult->result = CELL_SAVEDATA_CBRESULT_ERR_BROKEN;
		return;
	}

	set->reCreateMode = CELL_SAVEDATA_RECREATE_NO_NOBROKEN;
	set->setParam = 0;
	set->indicator = &s_indicator;

	cbResult->result = CELL_SAVEDATA_CBRESULT_OK_NEXT;
}

void GetAvailableSaveGamesTask::callbackLoadFile(CellSaveDataCBResult* cbResult, CellSaveDataFileGet* get, CellSaveDataFileSet* set)
{
	GetAvailableSaveGamesTask* this_ = static_cast< GetAvailableSaveGamesTask* >(cbResult->userdata);
	T_ASSERT (this_);

	cbResult->progressBarInc = 50;

	if (this_->m_loadBufferPending)
	{
		set->fileOperation = CELL_SAVEDATA_FILEOP_READ;
		set->fileType = CELL_SAVEDATA_FILETYPE_SECUREFILE;
		set->fileName = (char*)"ATT.BIN";
		set->fileOffset = 0;
		set->fileSize = this_->m_loadBuffer.size();
		set->fileBufSize = this_->m_loadBuffer.size();
		set->fileBuf = &this_->m_loadBuffer[0];
		set->reserved = 0;
		memcpy(set->secureFileId, s_secureFileId, CELL_SAVEDATA_SECUREFILEID_SIZE);

		cbResult->result = CELL_SAVEDATA_CBRESULT_OK_NEXT;

		this_->m_loadBufferPending = false;
	}
	else
	{
		DynamicMemoryStream dms(this_->m_loadBuffer, true, false);
		Ref< ISerializable > attachment = BinarySerializer(&dms).readObject();
		if (attachment)
		{
			this_->m_outSaveGames->push_back(new SaveGamePsn(L"", attachment));
			cbResult->result = CELL_SAVEDATA_CBRESULT_OK_LAST;
		}
		else
		{
			cbResult->result = CELL_SAVEDATA_CBRESULT_ERR_BROKEN;
		}
	}
}

	}
}
