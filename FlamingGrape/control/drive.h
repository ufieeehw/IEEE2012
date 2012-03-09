#ifndef DRIVE_H_
#define DRIVE_H_

enum DriveMode {
	DM_BANG,
	DM_TRAJ
};

void drive(float leftvel, float rightvel, DriveMode dm = DM_TRAJ);
void drive_dist(float leftvel, float rightvel, float ldist, float rdist, DriveMode dm = DM_TRAJ);
void drive_stop(DriveMode dm = DM_BANG);
void drive_off();

void drive_fd(float vel, DriveMode dm = DM_TRAJ);
void drive_fdDist(float vel, float dist, DriveMode dm = DM_TRAJ);
void drive_bk(float vel, DriveMode dm = DM_TRAJ);
void drive_bkDist(float vel, float dist, DriveMode dm = DM_TRAJ);
void drive_lturn(float vel, DriveMode dm = DM_TRAJ);
void drive_lturnDeg(float vel, float deg, DriveMode dm = DM_TRAJ);
void drive_rturn(float vel, DriveMode dm = DM_TRAJ);
void drive_rturnDeg(float vel, float deg, DriveMode dm = DM_TRAJ);

void drive_steer(float steer, float vel);

void drive_waitDist(float dist);

void drive_setTrajAmax(float amax);
float drive_getTrajAmax();

#endif
