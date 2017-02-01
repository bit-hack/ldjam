#pragma once
#include <array>
#include <cstdint>
#include <cmath>

namespace tengu {
struct mat4f_t {
	void identity( void ) {
		e[0x0]=1; e[0x1]=0; e[0x2]=0; e[0x3]=0;
		e[0x4]=0; e[0x5]=1; e[0x6]=0; e[0x7]=0;
		e[0x8]=0; e[0x9]=0; e[0xa]=1; e[0xb]=0;
		e[0xc]=0; e[0xd]=0; e[0xe]=0; e[0xf]=1;
	}

	void scale( float s ) {
		e[0x0]=s; e[0x1]=0; e[0x2]=0; e[0x3]=0;
		e[0x4]=0; e[0x5]=s; e[0x6]=0; e[0x7]=0;
		e[0x8]=0; e[0x9]=0; e[0xa]=s; e[0xb]=0;
		e[0xc]=0; e[0xd]=0; e[0xe]=0; e[0xf]=1;
	}

	void rotateX( float a ) {
		float s = sinf( a );
		float c = cosf( a );
		e[0x0]=1; e[0x1]=0; e[0x2]=0; e[0x3]=0;
		e[0x4]=0; e[0x5]=c; e[0x6]=s; e[0x7]=0;
		e[0x8]=0; e[0x9]=-s;e[0xa]=c; e[0xb]=0;
		e[0xc]=0; e[0xd]=0; e[0xe]=0; e[0xf]=1;
	}

	void rotateY( float a ) {
		float s = sinf( a );
		float c = cosf( a );
		e[0x0]=c; e[0x1]=0; e[0x2]=-s;e[0x3]=0;
		e[0x4]=0; e[0x5]=1; e[0x6]=0; e[0x7]=0;
		e[0x8]=s; e[0x9]=0; e[0xa]=c; e[0xb]=0;
		e[0xc]=0; e[0xd]=0; e[0xe]=0; e[0xf]=1;
	}

	void rotateZ( float a ) {
		float s = sinf( a );
		float c = cosf( a );
		e[0x0]=c; e[0x1]=s; e[0x2]=0; e[0x3]=0;
		e[0x4]=-s;e[0x5]=c; e[0x6]=0; e[0x7]=0;
		e[0x8]=0; e[0x9]=0; e[0xa]=1; e[0xb]=0;
		e[0xc]=0; e[0xd]=0; e[0xe]=0; e[0xf]=1;
	}

	template <typename vec3_t>
	void translate( const vec3_t &in ) {
		e[0x3] = in.x;
		e[0x7] = in.y;
		e[0xb] = in.z;
	}

	void projection(float fovy, float aspect, float znear, float zfar) {
		float r = (fovy / 2) * 57.295779f;
		float deltaZ = zfar - znear;
		float s = sinf(r);

		if (deltaZ == 0 || s == 0 || aspect == 0)
			return;

		float cotangent = cosf(r) / s;

		float i =  cotangent / aspect;
		float j =  cotangent;
		float k = -(zfar + znear) / deltaZ;
		float l = -1;
		float m = -2 * znear * zfar / deltaZ;
		float n =  0;

		e[0x0] = i;	e[0x1] = 0; e[0x2] = 0; e[0x3] = 0;
		e[0x4] = 0; e[0x5] =-j; e[0x6] = 0; e[0x7] = 0;
		e[0x8] = 0; e[0x9] = 0; e[0xa] = k; e[0xb] = l;
		e[0xc] = 0; e[0xd] = 0; e[0xe] = m; e[0xf] = n;
	}

    void isometric(float xscale, float yscale) {
        static const float sx = 2.f/xscale;
        static const float sy = 2.f/yscale;
        static const float cx = 0.f;
        static const float cy =-0.5f;
        static const float sz = 0.01f;
        e = std::array<float, 16>{
            +1.0f*sx, .5f*sy,  sz, 0.f,
            -1.0f*sx, .5f*sy,  sz, 0.f,
            +0.0f*sx, 1.f*sy, 0.f, 0.f,
            +1.0f*cx, 1.f*cy, 0.f, 1.f,
        };
    }

	float &operator [] (size_t i) {
		assert(i<e.size());
		return e[i];
	}

    const float operator [] (size_t i) const {
		assert(i<e.size());
		return e[i];
	}

	std::array<float, 16> e;
};
} // namespace tengu
