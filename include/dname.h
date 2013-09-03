/*
 * File:   dname.h
 * Author: mike
 *
 * Created on September 1, 2013, 1:51 PM
 */

#ifndef DNAME_H
#define	DNAME_H

#include <cstdint>

#define DNAME_LENGTH_MAX 16
#define DNAME_COUNT (4*26)

const char* dname(uint16_t id);

#endif	/* DNAME_H */

