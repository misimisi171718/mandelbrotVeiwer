uchar3 HsvToRgb(uchar3 hsv)
{
    uchar3 rgb;
    uchar region, remainder, p, q, t;

    if (hsv.z == 0)
    {
        rgb.x = hsv.y;
        rgb.y = hsv.y;
        rgb.z = hsv.y;
        return rgb;
    }

    region = hsv.x / 43;
    remainder = (hsv.x - (region * 43)) * 6; 

    p = (hsv.y * (255 - hsv.z)) >> 8;
    q = (hsv.y * (255 - ((hsv.z * remainder) >> 8))) >> 8;
    t = (hsv.y * (255 - ((hsv.z * (255 - remainder)) >> 8))) >> 8;

    switch (region)
    {
        case 0:
            rgb.x = hsv.y; rgb.y = t; rgb.z = p;
            break;
        case 1:
            rgb.x = q; rgb.y = hsv.y; rgb.z = p;
            break;
        case 2:
            rgb.x = p; rgb.y = hsv.y; rgb.z = t;
            break;
        case 3:
            rgb.x = p; rgb.y = q; rgb.z = hsv.y;
            break;
        case 4:
            rgb.x = t; rgb.y = p; rgb.z = hsv.y;
            break;
        default:
            rgb.x = hsv.y; rgb.y = p; rgb.z = q;
            break;
    }

    return rgb;
}

float3 calcSampl(float2 z,float2 c,size_t itCount)
{
    size_t i = 0;
    for(i = 0; i < itCount;i++)
    {
        float m = z.x;
        z.x = (z.x*z.x) - (z.y*z.y) + c.x;
        z.y = (2 * (m * z.y)) + c.y;
        if((z.x*z.x)+(z.y*z.y)>4)
            break;
    }
    float3 ret = {i,z.x,z.y};
    return ret;
}

kernel void mandelbrrot(
    __global uchar3* outdata,
    float2 offset,
    float2 step,
    float2 start,
    int itCount,
    int isJulia)
{
    size_t gi0 = get_global_id(0);
    size_t gi1 = get_global_id(1);
    float2 c;
    float2 z;
    if(isJulia)
    {
        z.x = (gi0 * step.x) + offset.x;
        z.y = (gi1 * step.y) + offset.y;
        c = start;
    }
    else
    {
        c.x = (gi0 * step.x) + offset.x;
        c.y = (gi1 * step.y) + offset.y;
        z = start;
    }

    float3 temp = calcSampl(z,c,itCount);
    size_t n = temp.x;
    z.x = temp.y;
    z.y = temp.z;

    size_t gi0s = get_global_size(0);
    if(n == itCount){
        outdata[(gi1 * gi0s) + gi0] = 0;
    }else{
        float x = (n - log2(log2((z.x*z.x)+(z.y*z.y))) + 4)*2;
        uchar3 z = {fmod(x,255),200,200};
        uchar3 y = HsvToRgb(z);
        outdata[(gi1 * gi0s) + gi0] = y;
    }
}
