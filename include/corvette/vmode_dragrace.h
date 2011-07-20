/*
 * vmode_dragrace.h
 *
 *  Created on: 15 Jun 2011
 *      Author: Hydra
 */

#ifndef VMODE_DRAGRACE_H_
#define VMODE_DRAGRACE_H_

#if defined(CONFIG_UNITTEST) || defined(CONFIG_DEBUG_DRAGRACE)
#define dragracedbprintf(format, ...) dbprintf(format, ##__VA_ARGS__)
#else
#define dragracedbprintf(format, ...)
#endif
extern U8 dragrace_starter_gid;
#define was_dragrace_started_by(gid) (dragrace_starter_gid == gid)

#endif /* VMODE_DRAGRACE_H_ */
