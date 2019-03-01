//
// @name git_hookd
// @date 2019/3/1
// @author xialeistudio<1065890063@qq.com>
//

#include <stdio.h>
#include <getopt.h>
#include <stdlib.h>
#include <semaphore.h>
#include <event.h>
#include <evhttp.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sys/types.h>

#define VERSION "1.0"

static const struct option long_options[] = {
    {"host", required_argument, NULL, 'l'},
    {"port", required_argument, NULL, 'p'},
    {"webroot", required_argument, NULL, 'w'},
    {"version", no_argument, NULL, 'v'},
    {"help", no_argument, NULL, 'h'},
    {0, 0, 0, 0}
};
// 信号量，防止并发执行命令
static sem_t *sem;
static char *host = "0.0.0.0", *webroot = NULL;
static int port = 10000;

void version() {
  printf(VERSION"\n");
}

void usage() {
  fprintf(stderr,
          "usage:\n"
              "  --webroot        webroot path. (eg \"/home/wwwroot\")\n"
              "  --port           listen port (default 10000)\n"
              "  --host           listen host (default \"0.0.0.0\")\n"
              "  -h, --help       show help\n"
              "  -v, --version    show version\n"
  );
}

void handle_request(struct evhttp_request *req, void *arg) {
  struct evkeyvalq query;
  evhttp_parse_query(req->uri, &query);
  // 读取项目名
  const char *name = evhttp_find_header(&query, "name");
  if (name == NULL) {
    evhttp_send_error(req, HTTP_BADREQUEST, "Bad Request");
    return;
  }
  // 检测项目目录
  char pathname[strlen(webroot) + strlen(name) + 1];
  strcpy(pathname, webroot);
  strcat(pathname, "/");
  strcat(pathname, name);
  if (access(pathname, F_OK) == -1) {
    evhttp_send_error(req, HTTP_BADREQUEST, "Bad Request: Project Not Found");
    return;
  }
  printf("pulling %s\n", pathname);
  // 等待信号防止并发更新
  sem_wait(sem);
  pid_t pid = fork();
  if (pid < 0) {
    evhttp_send_error(req, HTTP_INTERNAL, "Interval Error");
    fprintf(stderr, "fork failed");
    return;
  }
  if (pid == 0) {
    chdir(pathname);
    execlp("git", "git", "pull", NULL);
    exit(0);
  }
  waitpid(pid, NULL, 0);
  sem_post(sem);
  evhttp_send_reply(req, HTTP_OK, "OK", NULL);
}

int main(int argc, char **argv) {
  int opt;
  if (argc == 1) {
    usage();
    return 0;
  }
  // parse argv
  while ((opt = getopt_long_only(argc, argv, "lpw:vh", long_options, NULL)) != EOF) {
    switch (opt) {
      case 'l':host = optarg;
        break;
      case 'p':port = atoi(optarg);
        if (port <= 0) {
          fprintf(stderr, "invalid port %s\n", optarg);
          return 1;
        }
        break;
      case 'w':webroot = optarg;
        break;
      case 'v':version();
        return 0;
      case 'h':usage();
        return 0;
      default:usage();
        return 0;
    }
  }

  sem = sem_open("git-hookd", O_CREAT | O_EXCL, 0644, 1);
  event_init();
  struct evhttp *server = evhttp_start(host, (uint16_t) port);
  evhttp_set_gencb(server, handle_request, NULL);
  printf("server running on %s:%d. pid=%d webroot=%s\n", host, port, getpid(), webroot);
  event_dispatch();

  evhttp_free(server);
  sem_close(sem);
  return 0;
}