#ifndef PIDCONTROLLER_H
#define PIDCONTROLLER_H

#include <cfloat>
#include <cmath>
#include <cstdint>

class PIDController
{
  public:
	void update(uint64_t dt); // dt in µsec

	float setPoint             = FLT_MIN;
	float const* targetMeasure = nullptr;
	float* controlVariable     = nullptr;

	float Kp = FLT_MIN;
	float Ki = FLT_MIN;
	float Kd = FLT_MIN;

	float tol = 0;

  private:
	float lasterr = 0;
	float I       = 0;
	bool fixing   = false;
};

#endif // PIDCONTROLLER_H
