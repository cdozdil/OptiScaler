#include "XeSSFeature.h"

bool XeSSFeature::IsInited() 
{ 
	return _isInited; 
}

void XeSSFeature::SetInit(bool value) 
{ 
	_isInited = value; 
}