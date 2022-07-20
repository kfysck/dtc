#include <iostream>
#include <string>
#include <sstream>
#include "net_server.h"

#include "log.h"
#include "poll_thread.h"
#include "task_request.h"
#include "global.h"
#include "transaction_task.h"
#include "task_request.h"
#include "transaction_group.h"

extern CTransactionGroup* shardingsphere_group;

CNetServerProcess::CNetServerProcess(PollerBase * o) :
    CTaskDispatcher<CTaskRequest>(o),
    ownerThread(o),
    output(o)
{
}

CNetServerProcess::~CNetServerProcess()
{
}

void CNetServerProcess::TaskNotify(CTaskRequest * cur)
{
    log4cplus_debug("complex: packet receiving.");
    //there is a race condition here:
    //curr may be deleted during process (in task->ReplyNotify())
	CTransactionGroup* group = NULL;
	CTaskRequest * request = cur;
	int level = 0;

	if(request == NULL)
		return;

	//TODO: Parsing input, adapting thread groups.
	level = request->get_db_layer_level();
	log4cplus_debug("complex: packet db layer:%d.", level);
	group = shardingsphere_group;
	
	if(group->Push(request) != 0)
	{
		request->setResult("transaction insert queue failed.");
		request->ReplyNotify();
	}
	
    return ;
}

void CNetServerProcess::ReplyNotify(CTaskRequest* task) 
{
	m_mutex.lock();
	task->ReplyNotify();
	m_mutex.unlock();
}