#ifndef PARCH_NAME_H
#define	PARCH_NAME_H

#define NAME_LENGTH_MIN 1
#define NAME_LENGTH_MAX 40
#define NAME_DEFAULT "worker"

bool
validate(const char * str);
void
name_apply_default(char **ppname);
bool
negotiate(const char *request, const char *current);

#endif	/* PARCH_NAME_H */

