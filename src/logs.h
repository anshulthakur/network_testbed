/*
 * logs.h
 *
 *  Created on: 01-Mar-2017
 *      Author: anshul
 */

#ifndef LOGS_H_
#define LOGS_H_

#define TRACE_ENTRY()   fprintf(stdout, "%s {",__func__)
#define TRACE_EXIT()   fprintf(stdout, "%s }",__func__)

#define TRACE_INFO(...) fprintf(stdout, __VA_ARGS__)
#define TRACE_ERROR(...) fprintf(stderr, __VA_ARGS__)

#endif /* LOGS_H_ */
