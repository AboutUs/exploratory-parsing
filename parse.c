#include <signal.h>
#include <unistd.h>
#include <pthread.h>
#include <string.h>
#include "tally.h"
#include <stdio.h>
//#include <stdlib.h>

void done();

struct { int begin, end; } bindings[100];
void bind(int b);
void bind_end(int b);

void write_selected(char *buf, int len);

#define YY_DIE(args) fprintf(stderr, args); done(); abort();
#define YY_OUTPUT(args) write_selected args;
#define YY_noDEBUG 1

void YY_DOUBLE_BEGIN(char *rule) {push(rule);}
void YY_DOUBLE_END(char *rule, char *text) {pop(rule);}

void YY_TRIPLE_BEGIN(char *rule) {}
void YY_TRIPLE_END(char *rule, char *text) {
        if (strncmp(rule, "begin_", 6) == 0 || strncmp(rule, "start_", 6) == 0) {
                push(text);
        } else if (strncmp(rule, "end_", 4) == 0 || strncmp(rule, "stop_", 5) == 0) {
                pop(text);
        } else {
                pip(text);
        }
}

#include "parse.leg.c"
#include "tally.c"

FILE *binding_file;
FILE *selected_file;

void bind(int b) {
	bindings[b].begin = yythisthunk->mybegin;
	bindings[b].end = yythisthunk->myend;
}
void bind_end(int b) {
	bindings[b].end = yythisthunk->myend;
}
void unbind() {
	for (int i=0; i<=99; i++) {
		bindings[i].begin = -1;
		bindings[i].end = -1;
	}
}
void write_binding() {
	if ((bindings[0].begin!=-1) && (bindings[1].begin!=-1)) {
		int max;
		for (max=99; max>1; max--) {
			if (bindings[max].begin != -1) {
				break;
			}
		}
		for (int i=0; i<=max; i++) {
			if (bindings[i].begin == -1) {
				fprintf(binding_file, "\t");
			} else {
				yyText(bindings[i].begin, bindings[i].end); fprintf(binding_file, "%s%s", yytext, i!=max ? "\t" : "\n");
			}
		}
	}
	unbind();
}

void write_selected(char *buf, int len){
	if (issel()) {
		fwrite(buf, 1, len, selected_file);
		desel();
	}
}

int accepts = 0;

int depths[1000];
int maxdepth=0;

void *profile_thread(void *id) {
  while (1) {
    usleep(1000);
    int depth = yythunkpos;
    if (depth >= 1000) {
      depth = 999;
    }
    depths[depth]++;
    if (maxdepth < depth) {
      maxdepth = depth;
    }
  }
}

void profile_start() {
  for (int i=0; i<1000; i++) {
    depths[i]=0;
  }
  pthread_t clock;
  pthread_create(&clock, NULL, profile_thread, 0);
}

void progress () {
  fflush(stdout);
  FILE *fp = fopen("tally.txt", "w");
  fprintf(fp, "%llu\n", yyaccepted+yypos);
  fclose(fp);
  fflush(stdout);
  fp = fopen("profile.txt", "w");
  for (int i=0; i<=maxdepth; i++) {
    fprintf(fp, "%d ", depths[i]);
  }
  fprintf(fp, "\n");
  fclose(fp);
}

void done() {
  progress();
  dot();
  unlink("pid.txt");
	fclose(binding_file);
	fclose(selected_file);
}

void interrupt(int sig) {
  fprintf(stderr, "parse terminated by interrupt\n");
  done();
  exit(-1);
}

int main() {
  signal(SIGINT, interrupt);
  binding_file = fopen("bindings.txt", "w");
  selected_file = fopen("selected.txt", "w");
  unbind();
  profile_start();
  FILE *fp = fopen("pid.txt", "w");
  fprintf(fp, "%d\n", getpid());
  fclose(fp);
  while (yyparse()) {
		write_binding();
    if (!(++accepts % 1000)) {
      progress();
      dot();
    }
  }
  done();
  return 0;
}
