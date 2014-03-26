#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <zookeeper/zookeeper.h>
#include <zookeeper/zookeeper_log.h>

// global callback function, once exists method to be invoked,
// watcher would be set
void zktest_watcher_g(zhandle_t* zh, int type, int state,
        const char* path, void* watcherCtx)
{
    printf("Something happened.\n");
    printf("type: %d\n", type);
    printf("state: %d\n", state);
    printf("path: %s\n", path);
    printf("watcherCtx: %s\n", (char *)watcherCtx);
}

void zktest_dump_stat(const struct Stat *stat)
{
    char tctimes[40];
    char tmtimes[40];
    time_t tctime;
    time_t tmtime;

    if (!stat) {
        fprintf(stderr,"null\n");
        return;
    }
    tctime = stat->ctime/1000;
    tmtime = stat->mtime/1000;
       
    ctime_r(&tmtime, tmtimes);
    ctime_r(&tctime, tctimes);
       
    fprintf(stderr, "\tctime = %s\tczxid = %llx\n"
    "\tmtime = %s\tmzxid = %llx\n"
    "\tversion = %x\taversion = %x\n"
    "\tephemeralOwner = %llx\n",
     tctimes, stat->czxid,
     tmtimes, stat->mzxid,
    (unsigned int)stat->version, (unsigned int)stat->aversion,
    stat->ephemeralOwner);
}

void zktest_stat_completion(int rc, const struct Stat *stat, const void *data)
{
    fprintf(stderr, "[%s]: rc = %d Stat:\n", (char*)data, rc);
    zktest_dump_stat(stat);
}

void zktest_void_completion(int rc, const void *data)
{
    fprintf(stderr, "[%s]: rc = %d\n", (char*)(data==0?"null":data), rc);
}

void zktest_string_completion(int rc, const char *name, const void *data)
{
    fprintf(stderr, "[%s]: rc = %d\n", (char*)(data==0?"null":data), rc);
    if (!rc) {
        fprintf(stderr, "\tname = %s\n", name);
    }
}

int main(int argc, const char *argv[])
{
    const char* host = "127.0.0.1:2181,127.0.0.1:2182,"
        "127.0.0.1:2183";
    int timeout = 30000;
    
    zoo_set_debug_level(ZOO_LOG_LEVEL_WARN);
    zhandle_t* zkhandle = zookeeper_init(host,
            zktest_watcher_g, timeout, 0, "hello zookeeper.", 0);
    if (zkhandle == NULL) {
        fprintf(stderr, "Error when connecting to zookeeper servers...\n");
        exit(EXIT_FAILURE);
    }
		getchar();
    // struct ACL ALL_ACL[] = {{ZOO_PERM_ALL, ZOO_ANYONE_ID_UNSAFE}};
    // struct ACL_vector ALL_PERMS = {1, ALL_ACL};
    int ret = zoo_acreate(zkhandle, "/xyz", "hello", 5,
           &ZOO_OPEN_ACL_UNSAFE, 0 /* ZOO_SEQUENCE */,
           zktest_string_completion, "acreate");
    if (ret) {
        fprintf(stderr, "Error %d for %s\n", ret, "acreate");
        exit(EXIT_FAILURE);
    }
    ret = 0;
		getchar();

    ret = zoo_aexists(zkhandle, "/xyz", 1, zktest_stat_completion, "aexists");
    if (ret) {
        fprintf(stderr, "Error %d for %s\n", ret, "aexists");
        exit(EXIT_FAILURE);
    }
    ret = 0;
    // Wait for asynchronous zookeeper call done.
    getchar();

		ret = zoo_aexists(zkhandle, "/xyz", 1, zktest_stat_completion, "aexists");
		if (ret) {
			fprintf(stderr, "Error %d for %s\n", ret, "aexists");
			exit(EXIT_FAILURE);
		}
		ret = 0;
		getchar();

    ret = zoo_adelete(zkhandle, "/xyz", -1, zktest_void_completion, "adelete");
    if (ret) {
        fprintf(stderr, "Error %d for %s\n", ret, "adelete");
        exit(EXIT_FAILURE);
    }
    // Wait for asynchronous zookeeper call done.
    getchar();

    zookeeper_close(zkhandle);
}
