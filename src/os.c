#include "os.h"

#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <dirent.h>

#include "pair.h"
#include "error.h"
#include "string.h"
#include "number.h"
#include "env.h"
#include "symbol.h"
#include "primitive.h"

object_t scm_exit(vm_t *vm, size_t n, object_t args[]) {
  if (n == 1) {
    long code = scm_fixnum(args[0]);
    exit((int) code);
  } else {
    exit(0);
  }
}

object_t scm_pwd(vm_t *vm, size_t n, object_t args[]) {
  char cwd[2048];

  if (getcwd(cwd, sizeof(cwd)) != NULL) {
    size_t n = strlen(cwd);
    return make_string(vm, n, cwd);
  } else {
    return NULL;
  }
}

object_t scm_cd(vm_t *vm, size_t n, object_t args[]) {
  if (n != 1) {
    return make_error(vm, "cd: incorrect argument count", NULL);
  }

  if (scm_type(args[0]) != STRING) {
    return make_error(vm, "cd: path not string", args[0]);
  }

  char *path = string_cstr(args[0]);

  if (chdir(path) == 0) {
    return t;
  } else {
    return f;
  }
}

object_t scm_ls(vm_t *vm, size_t n, object_t args[]) {
  object_t list = NULL;
  DIR *dirp = opendir(".");
  struct dirent *dp;
  while ((dp = readdir(dirp)) != NULL) {
    object_t name = make_string(vm, strlen(dp->d_name), dp->d_name);
    list = cons(vm, name, list);
  }
  closedir(dirp);
  return list;
}

void define_os(vm_t *vm, object_t env) {
  def("exit", scm_exit)
  def("pwd", scm_pwd)
  def("cd", scm_cd)
  def("ls", scm_ls)
}
