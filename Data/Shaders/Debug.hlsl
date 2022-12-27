struct VS_INPUT
{
    float2 position : POSITION;
};

struct VS_OUTPUT
{
    float4 position : SV_POSITION;
};

VS_OUTPUT VS(in VS_INPUT IN)
{
    VS_OUTPUT OUT;

    OUT.position = float4(IN.position, 0.0, 1.0);

    return OUT;
};

float4 PS(in VS_OUTPUT IN) : COLOR
{
    return float4(1.0, 0.0, 0.0, 1.0);
};