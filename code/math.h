#if !defined(MATH_H)
union v2
{
    struct
    {
        r32 x,y;
    };
    struct
    {
        u,v;
    };
    real32 E[2];
}

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
        real32 Ignored0_;
    };
    struct
    {
        real32 Ignored1_;
        v2 yz;
    };
    struct
    {
        v2 uv;
        real32 Ignored2_;
    };
    struct
    {
        real32 Ignored3_;
        v2 vw;
    };
    r32 E[3];
};

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
        real32 Ignored0_;
        real32 Ignored1_;
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

#define MATH_H
#endif
