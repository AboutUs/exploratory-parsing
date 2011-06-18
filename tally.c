#include <string.h>

int selected = 0;

void sel() {
	selected = 1;
}

void desel() {
	selected = 0;
}

int issel() {
	return selected;
}

char *tag[1000];
int found=0;

char *intern (char *string) {
	for (int i=0; i<found; i++) {
		if (!strcmp(string, tag[i])) {
			return tag[i];
		}
	}
	if (found >= 1000) {
		YY_DIE("tally: intern: too many unique strings\n");
	}
	tag[found] = strdup(string);
	return tag[found++];
}

struct tally {
	char *parent, *child;
	int count, samples, period;
	long long int offsets[10];
	int lengths[10];
} arc[1000];
int total=0, uniq=0;

void dot () {
	FILE *fp = fopen("tally.dot", "w");
	fprintf(fp, "digraph xml {\n");
	fprintf(fp, "node [fillcolor=gold, style=filled, fontname=\"Monaco\"];\n");
	for (int i=0; i<uniq; i++) {
		fprintf(fp, "\"%s\" -> \"%s\" [ label = \"%d\", fontname=\"Monaco\",  URL=\"javascript:top.samples([", arc[i].parent, arc[i].child, arc[i].count);
		for (int j=0; j<arc[i].samples; j++) {
			fprintf(fp, "'%llu-%d'", arc[i].offsets[j], arc[i].lengths[j]);
			if (j < (arc[i].samples - 1)) {
				fprintf(fp, ",");
			}
		}
		fprintf(fp, "]);\"];\n");
	}
	fprintf(fp, "}\n");
	fclose(fp);
}

void degrade (struct tally *s) {
	for (int i=0, j=0; i<s->samples; i++, i++, j++) {
		s->offsets[j] = s->offsets[i];
		s->lengths[j] = s->lengths[i];
	}
	s->samples = s->samples / 2;
	s->period = s->period * 2;
}

void aggrade (struct tally *s) {
	if (s->samples >= 10) {
		degrade (s);
	} else {
		s->offsets[s->samples] = yyaccepted+yythisthunk->mybegin;
		s->lengths[s->samples] = yythisthunk->myend > yythisthunk->mybegin ? yythisthunk->myend - yythisthunk->mybegin : 0;
		s->samples++;
		sel();
	}
}

void regrade (struct tally *s) {
	// find sample (how? assume last, which won't survive degrade)
	int i = s->samples-1;
	// adjust length based on new yythisthunk
	int begin = ((long long)s->offsets[i])-yyaccepted;
	if (yythisthunk->myend > begin+4000) return;
	s->lengths[i] = yythisthunk->myend > begin ? yythisthunk->myend - begin : 0;
}

int add (char *p, char *c) {
	for (int i=0; i<uniq; i++) {
		if (arc[i].parent == p && arc[i].child == c) {
			arc[i].count++;
			if (!(arc[i].count % arc[i].period)) {
				aggrade (&arc[i]);
				return i;
			}
			return -1;
		}
	}
	if (uniq >= 1000) {
		YY_DIE("tally: add: too many arcs\n");
	}
	arc[uniq].parent = p;
	arc[uniq].child = c;
	arc[uniq].count = 1;
	arc[uniq].samples = 0;
	arc[uniq].period = 1;
	aggrade (&arc[uniq]);
	uniq++;
	return uniq-1;
}

char *nest[1000] = {"/root/"};
//int sediment[1000];
int depth = 1;

int limit = 3000;
int trace(int omit) {
        if (limit > 0) {
                limit--;
                for (int i = omit; i<depth; i++) {
                        fprintf(stderr, "   ");
                }
                return 1;
        }
        return 0;
}

void push (char *string) {
        // if (trace(0)) fprintf(stderr, "push %s\n", string);
	char *tag = intern(string);
	//sediment[depth] = add(nest[depth-1], tag);
	if (depth >= 1000) {
		YY_DIE("tally: push: too deeply nested\n");
	}
	nest[depth++] = tag;
}

void pop (char *string) {
        // if (trace(1)) fprintf(stderr, "pop %s\n", string);
	if (depth == 1) {
		return;
	}
	add(nest[depth-2], nest[depth-1]);
	if (strcmp(nest[--depth], string)) {
		pop(string);
	} else {
		//int i = sediment[depth];
		//if (i >= 0) {
		//	regrade (&arc[i]);
		//}
	}
}

void pip (char *string) {
	push(string);
	pop(string);
}

void pup () {
	--depth;
}
