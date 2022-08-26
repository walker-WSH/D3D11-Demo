
cbuffer MatrixBuffer
{
	matrix wvp;
};

struct VertexInputOutput
{
    float4 position : SV_POSITION; 
};
 
VertexInputOutput VS(VertexInputOutput input)
{
    VertexInputOutput output; 
    output.position = mul(float4(input.position.xyz, 1.f), wvp); 
    return output;
}