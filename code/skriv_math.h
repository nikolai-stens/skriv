#if !defined(SKRIV_MATH_H)

inline r32
Square(r32 A)
{
    r32 Result = A*A;

    return(Result);
}

inline r32
Clamp(r32 Min, r32 Value, r32 Max)
{
    r32 Result = Value;
    if(Result < Min)
    {
        Result = Min;
    }
    else if(Result > Max)
    {
        Result = Max;
    }

    return(Result);
}

inline r32
Clamp01(r32 Value)
{
    r32 Result = Clamp(0.0f, Value, 1.0f);

    return(Result);
}

//
// v2
//


union v2
{
    struct
    {
        r32 x,y;
    };
    struct
    {
        r32 u,v;
    };
    r32 E[2];
};

inline v2
V2(r32 X, r32 Y)
{
    v2 Result;
    Result.x = X;
    Result.y = Y;

    return(Result);
}

inline v2
Perp(v2 A)
{
    v2 Result = {-A.y, A.x};
    return(Result);
}


inline v2 
operator*(r32 A, v2 B)
{
    v2 Result;

    Result.x = A*B.x;
    Result.y = A*B.y;

    return(Result);
}

inline v2
operator*(v2 B, r32 A)
{
    v2 Result = A * B;
    return(Result);
}


inline v2 &
operator*=(v2 &A, r32 B)
{
    A = B * A;
    return(A);
}

inline v2 
operator-(v2 A)
{
    v2 Result;

    Result.x = -A.x;
    Result.y = -A.y;

    return(Result);
}

inline v2 
operator+(v2 A, v2 B)
{
    v2 Result;

    Result.x = A.x + B.x;
    Result.y = A.y + B.y;

    return(Result);
}

inline v2 &
operator +=(v2 &A, v2 B)
{
    A = A + B;
    return(A);
}

inline v2 
operator-(v2 A, v2 B)
{
    v2 Result;

    Result.x = A.x - B.x;
    Result.y = A.y - B.y;

    return(Result);
}

inline v2
Hadamard(v2 A, v2 B)
{
    v2 Result = {A.x*B.x, A.y*B.y};

    return(Result);
}

inline r32
Inner(v2 A, v2 B)
{
    r32 Result = A.x*B.x + A.y*B.y;

    return(Result);
}

inline r32
LengthSq(v2 A)
{
    r32 Result = Inner(A,A);

    return(Result);
}

inline v2 
Clamp01(v2 Value)
{
    v2 Result;
    Result.x = Clamp01(Value.x);
    Result.y = Clamp01(Value.y);

    return(Result);
}

inline v2
V2i(u32 X, u32 Y)
{
    v2 Result = {(r32)X, (r32)Y};

    return(Result);
}

//
// v3
//

union v3
{
    struct
    {
        r32 x, y, z;
    };
    struct
    {
        r32 u, v, w;
    };
    struct
    {
        r32 r, g, b;
    };
    struct
    {
        v2 xy;
        r32 Ignored0_;
    };
    struct
    {
        r32 Ignored1_;
        v2 yz;
    };
    struct
    {
        v2 uv;
        r32 Ignored2_;
    };
    struct
    {
        r32 Ignored3_;
        v2 vw;
    };
    r32 E[3];
};

inline v3 
operator*(r32 A, v3 B)
{
    v3 Result;

    Result.x = A*B.x;
    Result.y = A*B.y;
    Result.z = A*B.z;

    return(Result);
}

inline v3
operator*(v3 B, r32 A)
{
    v3 Result = A * B;
    return(Result);
}


inline v3 &
operator*=(v3 &A, r32 B)
{
    A = B * A;
    return(A);
}

inline v3 
operator-(v3 A)
{
    v3 Result;

    Result.x = -A.x;
    Result.y = -A.y;
    Result.z = -A.z;

    return(Result);
}

inline v3 
operator+(v3 A, v3 B)
{
    v3 Result;

    Result.x = A.x + B.x;
    Result.y = A.y + B.y;
    Result.z = A.z + B.z;

    return(Result);
}

inline v3 &
operator +=(v3 &A, v3 B)
{
    A = A + B;
    return(A);
}

inline v3 
operator-(v3 A, v3 B)
{
    v3 Result;

    Result.x = A.x - B.x;
    Result.y = A.y - B.y;
    Result.z = A.z - B.z;

    return(Result);
}

inline v3
Hadamard(v3 A, v3 B)
{
    v3 Result = {A.x*B.x, A.y*B.y, A.z*B.z};

    return(Result);
}

inline r32
Inner(v3 A, v3 B)
{
    r32 Result = A.x*B.x + A.y*B.y + A.z*B.z;

    return(Result);
}

inline r32
LengthSq(v3 A)
{
    r32 Result = Inner(A,A);

    return(Result);
}

inline v3 
Clamp01(v3 Value)
{
    v3 Result;
    Result.x = Clamp01(Value.x);
    Result.y = Clamp01(Value.y);
    Result.z = Clamp01(Value.z);

    return(Result);
}

inline v3
Lerp(v3 A, r32 t, v3 B)
{
    v3 Result = (1.0f - t)*A + t*B;

    return(Result);
}

//
// v4
//
union v4
{
    struct
    {
        union
        {
            v3 xyz;
            struct
            {
                r32 x, y, z;
            };
        };
        r32 w;
    };
    struct
    {
        union
        {
            v3 rgb;
            struct
            {
                r32 r, g, b;
            };
        };
        r32 a;
    };
    struct
    {
        v2 xy;
        r32 Ignored0_;
        r32 Ignored1_;
    };
    struct
    {
        r32 Ignored1_;
        v2 yz;
        r32 Ignored2_;
    };
    struct
    {
        r32 Ignored3_;
        r32 Ignored4_;
        v2 zw;
    };
    r32 E[4];
};

inline v4
V4(r32 X, r32 Y, r32 Z, r32 W)
{
    v4 Result;
    Result.x = X;
    Result.y = Y;
    Result.z = Z;
    Result.w = W;
    return(Result);
}

inline v4 
operator*(r32 A, v4 B)
{
    v4 Result;

    Result.x = A*B.x;
    Result.y = A*B.y;
    Result.z = A*B.z;
    Result.w = A*B.w;

    return(Result);
}

inline v4
operator*(v4 B, r32 A)
{
    v4 Result = A * B;
    return(Result);
}


inline v4 &
operator*=(v4 &A, r32 B)
{
    A = B * A;
    return(A);
}

inline v4 
operator-(v4 A)
{
    v4 Result;

    Result.x = -A.x;
    Result.y = -A.y;
    Result.z = -A.z;
    Result.w = -A.w;

    return(Result);
}

inline v4 
operator+(v4 A, v4 B)
{
    v4 Result;

    Result.x = A.x + B.x;
    Result.y = A.y + B.y;
    Result.z = A.z + B.z;
    Result.w = A.w + B.w;

    return(Result);
}

inline v4 &
operator +=(v4 &A, v4 B)
{
    A = A + B;
    return(A);
}

inline v4 
operator-(v4 A, v4 B)
{
    v4 Result;

    Result.x = A.x - B.x;
    Result.y = A.y - B.y;
    Result.z = A.z - B.z;
    Result.w = A.w - B.w;

    return(Result);
}

inline v4
Hadamard(v4 A, v4 B)
{
    v4 Result = {A.x*B.x, A.y*B.y, A.z*B.z, A.w*B.w};

    return(Result);
}

inline r32
Inner(v4 A, v4 B)
{
    r32 Result = A.x*B.x + A.y*B.y + A.z*B.z + A.w*B.w;

    return(Result);
}

inline r32
LengthSq(v4 A)
{
    r32 Result = Inner(A,A);

    return(Result);
}

inline v4 
Clamp01(v4 Value)
{
    v4 Result;
    Result.x = Clamp01(Value.x);
    Result.y = Clamp01(Value.y);
    Result.z = Clamp01(Value.z);
    Result.w = Clamp01(Value.w);

    return(Result);
}

inline v4
Lerp(v4 A, r32 t, v4 B)
{
    v4 Result = (1.0f - t)*A + t*B;

    return(Result);
}

inline v4
HexToV4(u32 Hex)
{
    v4 Result;
    Result.r = (1/255.0f)*(r32)(*(&(u8)Hex + 2));
    Result.g = (1/255.0f)*(r32)(*(&(u8)Hex + 1));
    Result.b = (1/255.0f)*(r32)(*(&(u8)Hex + 0));
    Result.a = (1/255.0f)*(r32)(*(&(u8)Hex + 3));

    return(Result);
}

inline v4
SRGB255ToLinear1(v4 Color)
{
    //TODO: Må lese meg litt mer opp på hvordan dette funker
    v4 Result;

    r32 Inv255 = 1.0f / 255.0f;

    Result.r = Square(Inv255*Color.r);
    Result.g = Square(Inv255*Color.g);
    Result.b = Square(Inv255*Color.b);
    Result.a = Inv255*Color.a;

    return(Result);
}


inline v4
Linear1ToSRGB255(v4 Color)
{
    v4 Result;

    Result.r = 255.0f*SquareRoot(Color.r);
    Result.g = 255.0f*SquareRoot(Color.g);
    Result.b = 255.0f*SquareRoot(Color.b);
    Result.a = 255.0f*Color.a;

    return(Result);
}

#define SKRIV_MATH_H
#endif
