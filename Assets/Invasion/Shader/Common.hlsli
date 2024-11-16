
float4x4 Common_Inverse(float4x4 m)
{
    float m00 = m[0][0], m01 = m[0][1], m02 = m[0][2], m03 = m[0][3];
    float m10 = m[1][0], m11 = m[1][1], m12 = m[1][2], m13 = m[1][3];
    float m20 = m[2][0], m21 = m[2][1], m22 = m[2][2], m23 = m[2][3];
    float m30 = m[3][0], m31 = m[3][1], m32 = m[3][2], m33 = m[3][3];
    
    float v0 = m20 * m31 - m21 * m30;
    float v1 = m20 * m32 - m22 * m30;
    float v2 = m20 * m33 - m23 * m30;
    float v3 = m21 * m32 - m22 * m31;
    float v4 = m21 * m33 - m23 * m31;
    float v5 = m22 * m33 - m23 * m32;

    float t00 = (v5 * m11 - v4 * m12 + v3 * m13);
    float t10 = -(v5 * m10 - v2 * m12 + v1 * m13);
    float t20 = (v4 * m10 - v2 * m11 + v0 * m13);
    float t30 = -(v3 * m10 - v1 * m11 + v0 * m12);

    float det = t00 * m00 + t10 * m01 + t20 * m02 + t30 * m03;
    
    if (abs(det) < 1e-6f)
    {
        return float4x4(
            1, 0, 0, 0,
            0, 1, 0, 0,
            0, 0, 1, 0,
            0, 0, 0, 1
        );
    }

    float invDet = 1.0f / det;

    float4x4 inverse;

    inverse[0][0] = t00 * invDet;
    inverse[0][1] = -(v5 * m01 - v4 * m02 + v3 * m03) * invDet;
    inverse[0][2] = (v5 * m31 - v4 * m32 + v3 * m33) * invDet;
    inverse[0][3] = -(v5 * m21 - v4 * m22 + v3 * m23) * invDet;

    inverse[1][0] = t10 * invDet;
    inverse[1][1] = (v5 * m00 - v2 * m02 + v1 * m03) * invDet;
    inverse[1][2] = -(v5 * m30 - v2 * m32 + v1 * m33) * invDet;
    inverse[1][3] = (v5 * m20 - v2 * m22 + v1 * m23) * invDet;

    inverse[2][0] = t20 * invDet;
    inverse[2][1] = -(v4 * m00 - v2 * m01 + v0 * m03) * invDet;
    inverse[2][2] = (v4 * m30 - v2 * m31 + v0 * m33) * invDet;
    inverse[2][3] = -(v4 * m20 - v2 * m21 + v0 * m23) * invDet;

    inverse[3][0] = t30 * invDet;
    inverse[3][1] = (v3 * m00 - v1 * m01 + v0 * m02) * invDet;
    inverse[3][2] = -(v3 * m30 - v1 * m31 + v0 * m32) * invDet;
    inverse[3][3] = (v3 * m20 - v1 * m21 + v0 * m22) * invDet;

    return inverse;
}