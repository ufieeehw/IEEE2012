#ifndef DEPLOY_H
#define DEPLOY_H

void deploy_out(bool full=false);
void deploy_in(bool full=false);
void deploy_off();

bool deploy_getBeamBreak();

void deploy_start();
void deploy_stop();
bool deploy_isDone();

void deploy_tick();

#endif
