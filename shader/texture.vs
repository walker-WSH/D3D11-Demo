
cbuffer MatrixBuffer
{
	matrix wvp;
};

struct VertexInputOutput
{
    float4 position : SV_POSITION;
    float2 tex : TEXCOORD0;
};
 
VertexInputOutput VS(VertexInputOutput input)
{
    VertexInputOutput output; 
    output.position = mul(float4(input.position.xyz, 1.f), wvp);
	output.tex = input.tex;
    return output;
}