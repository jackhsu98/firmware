/*=====================================================================================================*/
/*=====================================================================================================*/
#include "QuadCopterConfig.h"
/*=====================================================================================================*/
/*=====================================================================================================*/
#define Kp 15.0f
#define Ki 0.020f//0.02f
/*=====================================================================================================*/
/*=====================================================================================================*/
float True_R, R, inv_R, N_Ax_g, N_Ay_g, N_Az_g, 
		Gyro_AngX, Gyro_AngY, Gyro_AngZ,
		Gyro_Rx, Gyro_Ry, Gyro_Rz,
		True_Rx, True_Ry, True_Rz,
		ACOS;
void AHRS_Init(Quaternion *pNumQ, EulerAngle *pAngE)
{
	pNumQ->q0 = 1.0f;
	pNumQ->q1 = 0.0f;
	pNumQ->q2 = 0.0f;
	pNumQ->q3 = 0.0f;

	pAngE->Pitch = 0.0f;
	pAngE->Roll  = 0.0f;
	pAngE->Yaw   = 0.0f;
	True_R = 0.0;
	R = 0.0;
	True_Rx= 0.0;
	True_Ry= 0.0;
	True_Rz= 0.0;
	N_Ax_g = 0.0;
	N_Ay_g = 0.0;
	N_Az_g = 0.0;
	Gyro_AngX = 0.0;
	Gyro_AngY = 0.0;
	Gyro_AngZ = 0.0;
	Gyro_Rx = 0.0;
	Gyro_Ry = 0.0;
	Gyro_Rz=0.0;

}
/*=====================================================================================================*/
/*=====================================================================================================*/
void AHRS_Update(void)
{
	float tempX = 0, tempY = 0;
	float Normalize;
	float gx, gy, gz;
// float hx, hy, hz;
// float wx, wy, wz;
// float bx, bz;
	float ErrX, ErrY, ErrZ;
	float AccX, AccY, AccZ;
	float GyrX, GyrY, GyrZ;
// float MegX, MegY, MegZ;
	float /*Mq11, Mq12, */Mq13,/* Mq21, Mq22, */Mq23,/* Mq31, Mq32, */Mq33;

	static float AngZ_Temp = 0.0f;
	static float exInt = 0.0f, eyInt = 0.0f, ezInt = 0.0f;

//   Mq11 = NumQ.q0*NumQ.q0 + NumQ.q1*NumQ.q1 - NumQ.q2*NumQ.q2 - NumQ.q3*NumQ.q3;
//   Mq12 = 2.0f*(NumQ.q1*NumQ.q2 + NumQ.q0*NumQ.q3);
	Mq13 = 2.0f * (NumQ.q1 * NumQ.q3 - NumQ.q0 * NumQ.q2);
//   Mq21 = 2.0f*(NumQ.q1*NumQ.q2 - NumQ.q0*NumQ.q3);
//   Mq22 = NumQ.q0*NumQ.q0 - NumQ.q1*NumQ.q1 + NumQ.q2*NumQ.q2 - NumQ.q3*NumQ.q3;
	Mq23 = 2.0f * (NumQ.q0 * NumQ.q1 + NumQ.q2 * NumQ.q3);
//   Mq31 = 2.0f*(NumQ.q0*NumQ.q2 + NumQ.q1*NumQ.q3);
//   Mq32 = 2.0f*(NumQ.q2*NumQ.q3 - NumQ.q0*NumQ.q1);
	Mq33 = NumQ.q0 * NumQ.q0 - NumQ.q1 * NumQ.q1 - NumQ.q2 * NumQ.q2 + NumQ.q3 * NumQ.q3;

	Normalize = invSqrtf(squa(Acc.TrueX) + squa(Acc.TrueY) + squa(Acc.TrueZ));
	AccX = Acc.TrueX * Normalize;
	AccY = Acc.TrueY * Normalize;
	AccZ = Acc.TrueZ * Normalize;

// Normalize = invSqrtf(squa(Meg.TrueX) + squa(Meg.TrueY) + squa(Meg.TrueZ));
// MegX = Meg.TrueX*Normalize;
// MegY = Meg.TrueY*Normalize;
// MegZ = Meg.TrueZ*Normalize;

	gx = Mq13;
	gy = Mq23;
	gz = Mq33;

// hx = MegX*Mq11 + MegY*Mq21 + MegZ*Mq31;
// hy = MegX*Mq12 + MegY*Mq22 + MegZ*Mq32;
// hz = MegX*Mq13 + MegY*Mq23 + MegZ*Mq33;

// bx = sqrtf(squa(hx) + squa(hy));
// bz = hz;

// wx = bx*Mq11 + bz*Mq13;
// wy = bx*Mq21 + bz*Mq23;
// wz = bx*Mq31 + bz*Mq33;

	ErrX = (AccY * gz - AccZ * gy)/* + (MegY*wz - MegZ*wy)*/;
	ErrY = (AccZ * gx - AccX * gz)/* + (MegZ*wx - MegX*wz)*/;
	ErrZ = (AccX * gy - AccY * gx)/* + (MegX*wy - MegY*wx)*/;

	exInt = exInt + ErrX * Ki;
	eyInt = eyInt + ErrY * Ki;
	ezInt = ezInt + ErrZ * Ki;

	GyrX = toRad(Gyr.TrueX);
	GyrY = toRad(Gyr.TrueY);
	GyrZ = toRad(Gyr.TrueZ);

	GyrX = GyrX + Kp * ErrX + exInt;
	GyrY = GyrY + Kp * ErrY + eyInt;
	GyrZ = GyrZ + Kp * ErrZ + ezInt;

	Quaternion_RungeKutta(&NumQ, GyrX, GyrY, GyrZ, SampleRateHelf);
	Quaternion_Normalize(&NumQ);
	Quaternion_ToAngE(&NumQ, &AngE);

	tempX    = (Mag.X * arm_cos_f32(Mag.EllipseSita) + Mag.Y * arm_sin_f32(Mag.EllipseSita)) / Mag.EllipseB;
	tempY    = (-Mag.X * arm_sin_f32(Mag.EllipseSita) + Mag.Y * arm_cos_f32(Mag.EllipseSita)) / Mag.EllipseA;
	AngE.Yaw = atan2f(tempX, tempY);

	AngE.Pitch = toDeg(AngE.Pitch);
	AngE.Roll  = toDeg(AngE.Roll);
	AngE.Yaw   = toDeg(AngE.Yaw) + 180.0f;

	/* 互補濾波 Complementary Filter */
#define CF_A 0.9f
#define CF_B 0.1f
	AngZ_Temp = AngZ_Temp + GyrZ * SampleRate;
	AngZ_Temp = CF_A * AngZ_Temp + CF_B * AngE.Yaw;

	if (AngZ_Temp > 360.0f)
		AngE.Yaw = AngZ_Temp - 360.0f;
	else if (AngZ_Temp < 0.0f)
		AngE.Yaw = AngZ_Temp + 360.0f;
	else
		AngE.Yaw = AngZ_Temp;
}
/*=====================================================================================================*/
/*=====================================================================================================*/
void ahrs_complementary_filter()
{
	
	R = sqrtf(Acc.TrueX*Acc.TrueX+Acc.TrueY*Acc.TrueY+Acc.TrueZ*Acc.TrueZ);
	inv_R=1.0/R;
	N_Ax_g=(Acc.TrueX)*inv_R;

	N_Ay_g=Acc.TrueY*inv_R;
	N_Az_g=Acc.TrueZ*inv_R;

	Gyro_AngX = (Gyr.TrueX) * 0.002 * 0.0174444545234626; //*3.232238159179688
	Gyro_AngY = (Gyr.TrueY) * 0.002 * 0.0174444545234626;
	Gyro_AngZ = (Gyr.TrueZ) * 0.002 * 0.0174444545234626;



//output_high(PIN_F6);
/*
	True_Ry = True_Ry*cos(Gyro_AngZ)-True_Rx*sin(Gyro_AngZ);
	True_Rx = True_Ry*sin(Gyro_AngZ)+True_Rx*cos(Gyro_AngZ);
	
	True_Rz = True_Ry*sin(Gyro_AngY)+True_Rz*cos(Gyro_AngY);
	True_Ry = True_Ry*cos(Gyro_AngY)-True_Rz*sin(Gyro_AngY);

	True_Rz = True_Rx*sin(Gyro_AngX)+True_Rz*cos(Gyro_AngX);
	True_Rx = True_Rx*cos(Gyro_AngX)-True_Rz*sin(Gyro_AngX);

*/
//output_low(PIN_F6);

	True_Ry = True_Ry-True_Rx*Gyro_AngZ;
	True_Rx = True_Ry*Gyro_AngZ+True_Rx;
	
	True_Rz = True_Ry*Gyro_AngX+True_Rz;
	True_Ry = True_Ry-True_Rz*Gyro_AngX;

	True_Rz = True_Rx*Gyro_AngY+True_Rz;
	True_Rx = True_Rx-True_Rz*Gyro_AngY;

	Gyro_Rx=True_Rx;
	Gyro_Ry=True_Ry;
	Gyro_Rz=True_Rz;
	#define ComplementAlpha 0.0014
	True_Rx = (1.0-ComplementAlpha)*(Gyro_Rx)+ComplementAlpha*(N_Ax_g);
	True_Ry = (1.0-ComplementAlpha)*(Gyro_Ry)+ComplementAlpha*(N_Ay_g);
	True_Rz = (1.0-ComplementAlpha)*(Gyro_Rz)+ComplementAlpha*(N_Az_g);

	True_R=sqrtf(True_Rx*True_Rx+True_Ry*True_Ry+True_Rz*True_Rz);
	True_Rx=True_Rx/True_R;
	True_Ry=True_Ry/True_R;
	True_Rz=True_Rz/True_R;

/*
	True_AngX=acos(abs(True_Rx)/True_R)*57.32484076433121-90.0;
		if(True_Rx<0){

			True_AngX= -True_AngX;
		}

	True_AngY=acos(abs(True_Ry)/True_R)*57.32484076433121-90.0;
		if(True_Ry<0){

			True_AngY= -True_AngY;
		}
*/

//  Using corrected equation

	AngE.Roll=atanf(True_Ry/True_Rz)*57.2957795;


	ACOS = (True_Rx)/True_R;

	AngE.Pitch=acosf(ACOS )*57.2957795-90.0;
		if(True_Rx<0){

			AngE.Pitch= AngE.Pitch;
		}


}