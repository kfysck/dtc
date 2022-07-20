#include "agent_listen_pkg.h"
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <stdio.h>
#include "task_request.h"
#include "config.h"
#include "poll_thread.h"
#include "poll_thread_group.h"
#include "transaction_group.h"
#include "log.h"
#include "pipetask.h"
#include "memcheck.h"
#include "agent_process.h"
#include "net_server.h"
#include "cm_load.h"
#include "context.h"
#include "waitqueue.h"
#include "transaction_group.h"

Context context;

int background = 0;

ConfigHelper  g_config;

static CAgentListenPkg *agentListener;
static CAgentProcess   *agentProcess;
CNetServerProcess *netserverProcess;

static PollerBase* workerThread;

CTransactionGroup* shardingsphere_group = NULL;

static int start_main_thread()
{
	workerThread = new PollerBase("complex");
	if (workerThread->InitializeThread () == -1)
		return -1;

	netserverProcess = new CNetServerProcess(workerThread);
	agentProcess = new CAgentProcess(workerThread);
	agentListener = new CAgentListenPkg();

	agentProcess->BindDispatcher(netserverProcess);	
	if(agentListener->Bind("*:2002/tcp", agentProcess) < 0)
		return -1;
		
	workerThread->RunningThread();
	agentListener->Run();

	return 0;
}

static void stop_main_thread()
{
	if(workerThread)
		workerThread->interrupt();
	
	DELETE(agentListener);
	DELETE(agentProcess);
	DELETE(netserverProcess);
	DELETE(workerThread);
}

int GetIdxVal(const char *key,
	const char * const * array, int nDefault)
{
	if(key == NULL)
		return nDefault;

	if(isdigit(key[0]))
	{
		return nDefault;
	}

	for(int i=0; array[i]; i++) {
		if(!strcasecmp(key, array[i]))
			return i;
	}
	return -1;
}

int init_config(void)
{
	//load cold and hot db config.
	if(g_config.load_dtc_config() == false)
	{
		log4cplus_error("load db config error");
		return -1;
	}

	return 0;
}

static int set_background_mode()
{
	int ret = background ? daemon (1, 0) : 0;

	return ret;
}

void cm_create_pid() {
	ofstream pid_file;
	pid_file.open(DEF_PID_FILE, ios::out | ios::trunc);
	if (pid_file.is_open()) {
		pid_file << getpid();
		pid_file.close();
	} else {
		log4cplus_error("open pid file error. file:%s, errno:%d, errmsg:%s.",
			DEF_PID_FILE, errno, strerror(errno));
	}
}

void cm_delete_pid() {
	unlink(DEF_PID_FILE);
}

void catch_signal(int32_t signal) {
	log4cplus_error("catch signal: %d.", signal);
	switch (signal) {
	case SIGTERM:
		context.stop_flag = true;
		log4cplus_error("catch a stop signal.");
		break;
	case SIGINT:
		exit(0);
		break;
	default:
		break;
	}
}

void register_signal()
{
	if (SIG_ERR == signal(SIGTERM, catch_signal))
		log4cplus_info("set stop signal handler error. errno:%d,errmsg:%s.", errno, strerror(errno));
	if (SIG_ERR == signal(SIGINT, catch_signal))
		log4cplus_info("set stop signal handler error. errno:%d,errmsg:%s.", errno, strerror(errno));
	signal(SIGPIPE, SIG_IGN);
}

int start_db_thread_group(DBHost* dbconfig)
{
	const int thread_num  = g_config.GetIntValue("TransThreadNum", 10);
	CTransactionGroup* group = NULL;
	log4cplus_debug("transaction thread count:%d", thread_num);

	group = new CTransactionGroup(thread_num);
	if(group->Initialize(dbconfig))
	{
		log4cplus_error("init thread group failed");
		return -1;
	}

	shardingsphere_group = group;
	group->RunningThread();

	return 0;
}

int main(int argc, char* argv[])
{
	int ret = 0;

	init_log4cplus();
	log4cplus_info("complex main entry.");

    if(init_config() == -1)
		return -1;

    if(set_background_mode () < 0)
    	return -1;

	register_signal();

	cm_create_pid();

	//shardingsphere database instance.
	int res = start_db_thread_group(&g_config.shardingsphere);
	if(res != 0)
	{
		log4cplus_error("start hotdb thread group faield, exit. %d", res);
		return -1;
	}

    start_main_thread();

    log4cplus_info("complex main running.");
    while(!context.stop_flag){
    	sleep(10);
    }
    log4cplus_info("complex main stoping.");
	
	stop_main_thread();

	cm_delete_pid();

	log4cplus_info("complex main end.");

    return ret;
}