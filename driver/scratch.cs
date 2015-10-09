    struct Vector3D 
    {
        public float X { get; }
        public float Y { get; }
        public float Z { get; }

        public Vector3D(float x, float y, float z)
        {
            X = x;
            Y = y;
            Z = z;
        }
    }

    struct Matrix3D
    {
        public float M11 { get; }
        public float M12 { get; }
        public float M13 { get; }
        public float M21 { get; }
        public float M22 { get; }
        public float M23 { get; }
        public float M31 { get; }
        public float M32 { get; }
        public float M33 { get; }

        public Matrix3D(float m11, float m12, float m13,
                        float m21, float m22, float m23,
                        float m31, float m32, float m33)
        {
            M11 = m11;
            M12 = m12;
            M13 = m13;
            M21 = m21;
            M22 = m22;
            M23 = m23;
            M31 = m31;
            M32 = m32;
            M33 = m33;
        }
    }
