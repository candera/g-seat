namespace driver
{
    public struct Parts<T>
    {
        public T BL;
        public T BR;
        public T SL;
        public T SR;
    }
    
    public struct Stats
    {
        public Parts<double> Forces;
        public Parts<double> G;
        public Parts<long> Commands;
        public double T;
        public double DeltaT;
        public double Yaw;
        public double Pitch;
        public double Roll;
        public Vector3D VAC; // Velocity of A/C frame
        public Vector3D DVAC; // Acceleration of A/C frame
        public Vector3D DVSeat; // Acceleration of Seat
        public Vector3D DVTot; // Total acceleration
    }
}
