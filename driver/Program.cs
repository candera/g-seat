using System;
using System.Collections.Concurrent;
using System.Collections.Generic;
using System.IO;
using System.IO.Ports;
using System.Linq;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading;
using System.Threading.Tasks;

namespace driver
{
    class NativeMethods
    {
        [Flags]
        public enum FileMapAccess : uint
        {
            FileMapCopy = 0x0001,
            FileMapWrite = 0x0002,
            FileMapRead = 0x0004,
            FileMapAllAccess = 0x001f,
            fileMapExecute = 0x0020,
        }

        [DllImport("kernel32.dll", SetLastError = true)]
        public static extern IntPtr OpenFileMapping(
                                                    FileMapAccess dwDesiredAccess,
                                                    bool bInheritHandle,
                                                    string lpName);

        [DllImport("kernel32.dll", SetLastError = true)]
        public static extern bool CloseHandle(IntPtr hHandle);

        [DllImport("kernel32.dll", SetLastError = true)]
        public static extern IntPtr MapViewOfFile(
                                                  IntPtr hFileMappingObject,
                                                  FileMapAccess dwDesiredAccess,
                                                  uint dwFileOffsetHigh,
                                                  uint dwFileOffsetLow,
                                                  uint dwNumberOfBytesToMap);

        [DllImport("kernel32.dll", SetLastError = true)]
        public static extern bool UnmapViewOfFile(IntPtr lpBaseAddress);
    }

    /// <summary>
    /// Utility class to create and use sharedmemory areas.
    /// </summary>
    public class SharedMemory
    {
        private bool _disposed;

        private string _sharedMemoryName;
        private int _checkValue;

        private IntPtr _sharedMemoryHandle = IntPtr.Zero;
        private IntPtr _sharedMemoryAddress = IntPtr.Zero;

        public SharedMemory(string sharedMemoryName)
        {
            _sharedMemoryName = sharedMemoryName;
        }

        public int CheckValue
        {
            get
            {
                return _checkValue;
            }
            set
            {
                _checkValue = value;
            }
        }

        public bool IsOpen
        {
            get
            {
                return (_sharedMemoryAddress != IntPtr.Zero);
            }
        }

        public bool IsDataAvailable
        {
            get
            {
                if (!IsOpen)
                {
                    Open();
                }

                if (IsOpen)
                {
                    int value = Marshal.ReadInt32(_sharedMemoryAddress);
                    if (value != CheckValue)
                    {
                        return true;
                    }
                }
                return false;
            }
        }

        public object MarshalTo(Type type)
        {
            return Marshal.PtrToStructure(_sharedMemoryAddress, type);
        }

        public object MarshalTo(Type type, Int64 offset)
        {
            return Marshal.PtrToStructure(GetPointer(offset), type);
        }

        public IntPtr GetPointer()
        {
            return _sharedMemoryAddress;
        }

        public IntPtr GetPointer(Int64 offset)
        {
            return new IntPtr(_sharedMemoryAddress.ToInt64() + offset);
        }

        public bool Open()
        {
            if (_sharedMemoryAddress != IntPtr.Zero)
            {
                return true;
            }

            if (_sharedMemoryHandle == IntPtr.Zero)
            {
                _sharedMemoryHandle = NativeMethods.OpenFileMapping(NativeMethods.FileMapAccess.FileMapRead, false, _sharedMemoryName);
            }

            if (_sharedMemoryHandle != IntPtr.Zero)
            {
                _sharedMemoryAddress = NativeMethods.MapViewOfFile(_sharedMemoryHandle, NativeMethods.FileMapAccess.FileMapRead, 0, 0, 0);

                if (_sharedMemoryAddress != IntPtr.Zero)
                {
                    return true;
                }
            }

            return false;
        }

        public void Close()
        {
            if (_sharedMemoryAddress != IntPtr.Zero)
            {
                NativeMethods.UnmapViewOfFile(_sharedMemoryAddress);
                _sharedMemoryAddress = IntPtr.Zero;
            }
            if (_sharedMemoryHandle != IntPtr.Zero)
            {
                NativeMethods.CloseHandle(_sharedMemoryHandle);
                _sharedMemoryHandle = IntPtr.Zero;
            }
        }

        #region IDisposable Members

        public void Dispose()
        {
            Dispose(true);
            GC.SuppressFinalize(this);
        }

        protected virtual void Dispose(bool disposing)
        {
            if (!_disposed)
            {
                if (disposing)
                {
                    Close();
                }
            }
            _disposed = true;
        }

        ~SharedMemory()
        {
            Dispose(false);
        }

        #endregion
    }

    [StructLayout(LayoutKind.Sequential, Pack = 1)]
    struct TextLines
    {
        [MarshalAs(UnmanagedType.ByValTStr, SizeConst = 26)]
        public string line1;
        [MarshalAs(UnmanagedType.ByValTStr, SizeConst = 26)]
        public string line2;
        [MarshalAs(UnmanagedType.ByValTStr, SizeConst = 26)]
        public string line3;
        [MarshalAs(UnmanagedType.ByValTStr, SizeConst = 26)]
        public string line4;
        [MarshalAs(UnmanagedType.ByValTStr, SizeConst = 26)]
        public string line5;
    }

    [Flags]
    enum BMSLightBits : uint
    {
        MasterCaution = 0x1,  // Left eyebrow

        // Brow Lights
        TF = 0x2,   // Left eyebrow
        OXY_BROW = 0x4,   //  repurposed for eyebrow OXY LOW (was OBS, unused)
        EQUIP_HOT = 0x8,   // Caution light; repurposed for cooling fault (was: not used)
        WOW = 0x10,  // True if weight is on wheels: this is not a lamp bit!
        ENG_FIRE = 0x20,  // Right eyebrow; upper half of split face lamp
        CONFIG = 0x40,  // Stores config, caution panel
        HYD = 0x80,  // Right eyebrow; see also OIL (this lamp is not split face)
        Flcs_ABCD = 0x100, // TEST panel FLCS channel lamps; repurposed, was OIL (see HYD; that lamp is not split face)
        FLCS = 0x200, // Right eyebrow; was called DUAL which matches block 25, 30/32 and older 40/42
        CAN = 0x400, // Right eyebrow
        T_L_CFG = 0x800, // Right eyebrow

        // AOA Indexers
        AOAAbove = 0x1000,
        AOAOn = 0x2000,
        AOABelow = 0x4000,

        // Refuel/NWS
        RefuelRDY = 0x8000,
        RefuelAR = 0x10000,
        RefuelDSC = 0x20000,

        // Caution Lights
        FltControlSys = 0x40000,
        LEFlaps = 0x80000,
        EngineFault = 0x100000,
        Overheat = 0x200000,
        FuelLow = 0x400000,
        Avionics = 0x800000,
        RadarAlt = 0x1000000,
        IFF = 0x2000000,
        ECM = 0x4000000,
        Hook = 0x8000000,
        NWSFail = 0x10000000,
        CabinPress = 0x20000000,

        AutoPilotOn = 0x40000000,  // TRUE if is AP on.  NB: This is not a lamp bit!
        TFR_STBY = 0x80000000,  // MISC panel; lower half of split face TFR lamp

    }

    [Flags]
    enum LightBits2 : uint
    {
        // Threat Warning Prime
        HandOff = 0x1,
        Launch = 0x2,
        PriMode = 0x4,
        Naval = 0x8,
        Unk = 0x10,
        TgtSep = 0x20,

        // EWS
        Go = 0x40,              // On and operating normally
        NoGo = 0x80,     // On but malfunction present
        Degr = 0x100,    // Status message: AUTO DEGR
        Rdy = 0x200,    // Status message: DISPENSE RDY
        ChaffLo = 0x400,    // Bingo chaff quantity reached
        FlareLo = 0x800,    // Bingo flare quantity reached

        // Aux Threat Warning
        AuxSrch = 0x1000,
        AuxAct = 0x2000,
        AuxLow = 0x4000,
        AuxPwr = 0x8000,

        // ECM
        EcmPwr = 0x10000,
        EcmFail = 0x20000,

        // Caution Lights
        FwdFuelLow = 0x40000,
        AftFuelLow = 0x80000,

        EPUOn = 0x100000,  // EPU panel; run light
        JFSOn = 0x200000,  // Eng Jet Start panel; run light

        // Caution panel
        SEC = 0x400000,
        OXY_LOW = 0x800000,
        PROBEHEAT = 0x1000000,
        SEAT_ARM = 0x2000000,
        BUC = 0x4000000,
        FUEL_OIL_HOT = 0x8000000,
        ANTI_SKID = 0x10000000,

        TFR_ENGAGED = 0x20000000,  // MISC panel; upper half of split face TFR lamp
        GEARHANDLE = 0x40000000,  // Lamp in gear handle lights on fault or gear in motion
        ENGINE = 0x80000000,  // Lower half of right eyebrow ENG FIRE/ENGINE lamp
    };

    [Flags]
    enum BMSLightBits3 : uint
    {
        // Elec panel
        FlcsPmg = 0x1,
        MainGen = 0x2,
        StbyGen = 0x4,
        EpuGen = 0x8,
        EpuPmg = 0x10,
        ToFlcs = 0x20,
        FlcsRly = 0x40,
        BatFail = 0x80,

        // EPU panel
        Hydrazine = 0x100,
        Air = 0x200,

        // Caution panel
        Elec_Fault = 0x400,
        Lef_Fault = 0x800,

        OnGround = 0x1000,   // weight-on-wheels
        FlcsBitRun = 0x2000,   // FLT CONTROL panel RUN light (used to be Multi-engine fire light)
        FlcsBitFail = 0x4000,   // FLT CONTROL panel FAIL light (used to be Lock light Cue; non-F-16)
        DbuWarn = 0x8000,   // Right eyebrow DBU ON cell; was Shoot light cue; non-F16
        NoseGearDown = 0x10000,  // Landing gear panel; on means down and locked
        LeftGearDown = 0x20000,  // Landing gear panel; on means down and locked
        RightGearDown = 0x40000,  // Landing gear panel; on means down and locked
        ParkBrakeOn = 0x100000, // Parking brake engaged; NOTE: not a lamp bit
        Power_Off = 0x200000, // Set if there is no electrical power.  NB: not a lamp bit

        // Caution panel
        cadc = 0x400000,

        // Left Aux console
        SpeedBrake = 0x800000,  // True if speed brake is in anything other than stowed position
    }

    [Flags]
    enum HsiBits : uint
    {
        ToTrue = 0x01,    // HSI_FLAG_TO_TRUE
        IlsWarning = 0x02,    // HSI_FLAG_ILS_WARN
        CourseWarning = 0x04,    // HSI_FLAG_CRS_WARN
        Init = 0x08,    // HSI_FLAG_INIT
        TotalFlags = 0x10,    // HSI_FLAG_TOTAL_FLAGS; never set
        ADI_OFF = 0x20,    // ADI OFF Flag
        ADI_AUX = 0x40,    // ADI AUX Flag
        ADI_GS = 0x80,    // ADI GS FLAG
        ADI_LOC = 0x100,   // ADI LOC FLAG
        HSI_OFF = 0x200,   // HSI OFF Flag
        BUP_ADI_OFF = 0x400,   // Backup ADI Off Flag
        VVI = 0x800,   // VVI OFF Flag
        AOA = 0x1000,  // AOA OFF Flag
        AVTR = 0x2000,  // AVTR Light
        OuterMarker = 0x4000,  // MARKER beacon light for outer marker
        MiddleMarker = 0x8000,  // MARKER beacon light for middle marker
        FromTrue = 0x10000, // HSI_FLAG_TO_TRUE == 2, FROM
    };

    [StructLayout(LayoutKind.Sequential, Pack = 1)]
    struct FlightData
    {
        public float x;            // Ownship North (Ft)
        public float y;            // Ownship East (Ft)
        public float z;            // Ownship Down (Ft)
        public float xDot;         // Ownship North Rate (ft/sec)
        public float yDot;         // Ownship East Rate (ft/sec)
        public float zDot;         // Ownship Down Rate (ft/sec)
        public float alpha;        // Ownship AOA (Degrees)
        public float beta;         // Ownship Beta (Degrees)
        public float gamma;        // Ownship Gamma (Radians)
        public float pitch;        // Ownship Pitch (Radians)
        public float roll;         // Ownship Pitch (Radians)
        public float yaw;          // Ownship Pitch (Radians)
        public float mach;         // Ownship Mach number
        public float kias;         // Ownship Indicated Airspeed (Knots)
        public float vt;           // Ownship True Airspeed (Ft/Sec)
        public float gs;           // Ownship Normal Gs
        public float windOffset;   // Wind delta to FPM (Radians)
        public float nozzlePos;    // Ownship engine nozzle percent open (0-100)
        public float internalFuel; // Ownship internal fuel (Lbs)
        public float externalFuel; // Ownship external fuel (Lbs)
        public float fuelFlow;     // Ownship fuel flow (Lbs/Hour)
        public float rpm;          // Ownship engine rpm (Percent 0-103)
        public float ftit;         // Ownship Forward Turbine Inlet Temp (Degrees C)
        public float gearPos;      // Ownship Gear position 0 = up, 1 = down;
        public float speedBrake;   // Ownship speed brake position 0 = closed, 1 = 60 Degrees open
        public float epuFuel;      // Ownship EPU fuel (Percent 0-100)
        public float oilPressure;  // Ownship Oil Pressure (Percent 0-100)
        public BMSLightBits lightBits;    // Cockpit Indicator Lights, one bit per bulb. See enum

        // These are inputs. Use them carefully
        // NB: these do not work when TrackIR device is enabled
        public float headPitch;    // Head pitch offset from design eye (radians)
        public float headRoll;     // Head roll offset from design eye (radians)
        public float headYaw;      // Head yaw offset from design eye (radians)

        // new lights
        public LightBits2 lightBits2;   // Cockpit Indicator Lights, one bit per bulb. See enum
        public BMSLightBits3 lightBits3;   // Cockpit Indicator Lights, one bit per bulb. See enum

        // chaff/flare
        public float ChaffCount;   // Number of Chaff left
        public float FlareCount;   // Number of Flare left

        // landing gear
        public float NoseGearPos;  // Position of the nose landinggear; caution: full down values defined in dat files
        public float LeftGearPos;  // Position of the left landinggear; caution: full down values defined in dat files
        public float RightGearPos; // Position of the right landinggear; caution: full down values defined in dat files

        // ADI values
        public float AdiIlsHorPos; // Position of horizontal ILS bar
        public float AdiIlsVerPos; // Position of vertical ILS bar

        // HSI states
        public int courseState;    // HSI_STA_CRS_STATE
        public int headingState;   // HSI_STA_HDG_STATE
        public int totalStates;    // HSI_STA_TOTAL_STATES; never set

        // HSI values
        public float courseDeviation;  // HSI_VAL_CRS_DEVIATION
        public float desiredCourse;    // HSI_VAL_DESIRED_CRS *
        public float distanceToBeacon;    // HSI_VAL_DISTANCE_TO_BEACON *
        public float bearingToBeacon;  // HSI_VAL_BEARING_TO_BEACON *
        public float currentHeading;      // HSI_VAL_CURRENT_HEADING *
        public float desiredHeading;   // HSI_VAL_DESIRED_HEADING *
        public float deviationLimit;      // HSI_VAL_DEV_LIMIT
        public float halfDeviationLimit;  // HSI_VAL_HALF_DEV_LIMIT
        public float localizerCourse;     // HSI_VAL_LOCALIZER_CRS
        public float airbaseX;            // HSI_VAL_AIRBASE_X
        public float airbaseY;            // HSI_VAL_AIRBASE_Y
        public float totalValues;         // HSI_VAL_TOTAL_VALUES; never set

        public float TrimPitch;  // Value of trim in pitch axis, -0.5 to +0.5
        public float TrimRoll;   // Value of trim in roll axis, -0.5 to +0.5
        public float TrimYaw;    // Value of trim in yaw axis, -0.5 to +0.5

        // HSI flags
        public HsiBits hsiBits;      // HSI flags

        //DED Lines
        public TextLines DED;
        public TextLines DEDInverse;

        //PFL Lines
        public TextLines PFL;
        public TextLines PFLInverse;

        //TacanChannel
        public int UFCTChan;
        public int AUXTChan;

        // RWR
        public int RwrObjectCount;
        [MarshalAs(UnmanagedType.ByValArray, SizeConst = 40)]
        public int[] RWRsymbol;
        [MarshalAs(UnmanagedType.ByValArray, SizeConst = 40)]
        public float[] bearing;
        [MarshalAs(UnmanagedType.ByValArray, SizeConst = 40)]
        public uint[] missileActivity;
        [MarshalAs(UnmanagedType.ByValArray, SizeConst = 40)]
        public uint[] missileLaunch;
        [MarshalAs(UnmanagedType.ByValArray, SizeConst = 40)]
        public uint[] selected;
        [MarshalAs(UnmanagedType.ByValArray, SizeConst = 40)]
        public float[] lethality;
        [MarshalAs(UnmanagedType.ByValArray, SizeConst = 40)]
        public uint[] newDetection;

        //fuel values
        public float fwd;
        public float aft;
        public float total;

        public int VersionNum;

        float headX;        // Head X offset from design eye (feet)
        float headY;        // Head Y offset from design eye (feet)
        float headZ;        // Head Z offset from design eye (feet)

        int MainPower;
    }

    enum TacanSources : int
    {
        UFC = 0,
        AUX,
        NUMBER_OF_SOURCES
    };

    [Flags]
    enum TacanBits : byte
    {
        band = 0x01,   // true in this bit position if band is X
        mode = 0x02    // true in this bit position if domain is air to air
    };

    [StructLayout(LayoutKind.Sequential)]
    struct FlightData2
    {
        public float nozzlePos2;   // Ownship engine nozzle2 percent open (0-100)
        public float rpm2;         // Ownship engine rpm2 (Percent 0-103)
        public float ftit2;        // Ownship Forward Turbine Inlet Temp2 (Degrees C)
        public float oilPressure2; // Ownship Oil Pressure2 (Percent 0-100)
        public byte navMode;  // current mode selected for HSI/eHSI (added in BMS4)
        public float aauz; // Ownship barometric altitude given by AAU (depends on calibration)

        [MarshalAs(UnmanagedType.ByValArray, SizeConst = (int)TacanSources.NUMBER_OF_SOURCES)]
        public TacanBits[] tacanInfo;      // Tacan band/mode settings for UFC and AUX COMM

    }

    /* I made my own because I wrote this on a Mac and Mono
       doesn't have the Microsoft libs. */
    public struct Vector3D
    {
        public double X { get; }
        public double Y { get; }
        public double Z { get; }

        public Vector3D(double x, double y, double z)
        {
            X = x;
            Y = y;
            Z = z;
        }

        public bool Equals(Vector3D that)
        {
            return this.X == that.X && this.Y == that.Y && this.Z == that.Z;
        }
    }

    struct Matrix3D
    {
        public double M11 { get; }
        public double M12 { get; }
        public double M13 { get; }
        public double M21 { get; }
        public double M22 { get; }
        public double M23 { get; }
        public double M31 { get; }
        public double M32 { get; }
        public double M33 { get; }

        public Matrix3D(double m11, double m12, double m13,
                        double m21, double m22, double m23,
                        double m31, double m32, double m33)
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


    static class Program
    {
        public static double sin(double rad)
        {
            return (double) Math.Sin(rad);
        }

        public static double cos(double rad)
        {
            return (double) Math.Cos(rad);
        }

        public static double ToRadians(double deg)
        {
            //  (/ (* deg Math/PI) 180.0))
            return (double) (deg * 3.1415926F) / 180.0;
        }

        public static double ToDegrees(double rad)
        {
            //  (/ (* deg Math/PI) 180.0))
            return (double) (rad * 180.0) / 3.1415926;
        }

        public static double Dot(Vector3D a, Vector3D b)
        {
            return (double) (a.X * b.X + a.Y * b.Y + a.Z * b.Z);
        }

        public static Vector3D Transform(Matrix3D m, Vector3D v)
        {
            return new Vector3D(v.X * m.M11 + v.Y * m.M12 + v.Z * m.M13,
                                v.X * m.M21 + v.Y * m.M22 + v.Z * m.M23,
                                v.X * m.M31 + v.Y * m.M32 + v.Z * m.M33);
        }

        public static Vector3D Add(Vector3D a, Vector3D b)
        {
            return new Vector3D(a.X + b.X, a.Y + b.Y, a.Z + b.Z);
        }

        public static Vector3D Subtract(Vector3D a, Vector3D b)
        {
            return new Vector3D(a.X - b.X, a.Y - b.Y, a.Z - b.Z);
        }

        public static Vector3D Scale(Vector3D v, double s)
        {
            return new Vector3D(v.X * s, v.Y * s, v.Z * s);
        }

        /// <summary>
        ///   Returns a Matrix3D for the specified yaw, pitch, and
        ///   roll, which are in radians.
        /// </summary>
        static Matrix3D YPR(double yaw, double pitch, double roll)
        {
            double sy = sin(yaw);
            double cy = cos(yaw);
            double sp = sin(pitch);
            double cp = cos(pitch);
            double sr = sin(roll);
            double cr = cos(roll);

            return new Matrix3D(cp * cy,
                                (cy * sp * sr) - (sy * cr),
                                (cy * cr * sp) + (sy * sr),

                                sy * cp,
                                (sy * sp * sr) + (cr * cy),
                                (sy * cr * sp) - (cy * sr),

                                -sp,
                                sr * cp,
                                cr * cp);
        }

        static double Map(double val, double in1, double in2, double out1, double out2)
        {
            // TODO: Make this nonlinear if need be
            return ((val - in1) / (in2 - in1)) * (out2 - out1) + out1;
        }

        static double Clamp(double val, double min, double max)
        {
            return (double) Math.Max(Math.Min(val, max), min);
        }

        static double Nonlinear(double x)
        {
            return 1.0 - (1.0 / (x + 1.0));
        }

        public static long Position(double g, double baseG)
        {
            if (g > baseG)
            {
                return (long) Map(Nonlinear(g - baseG),
                                  0, 1, 2500, 10000);
            }
            else
            {
                return (long) Map(Nonlinear(baseG - g),
                                  0, 1, 2500, 0);
            }
        }

        static void Sleep(int millis)
        {
            System.Threading.Thread.Sleep(millis);
        }

        static long _startTicks = DateTime.Now.Ticks;
        /// <summary>
        ///   Time in seconds from some point
        /// </summary>
        static double Now()
        {
            return (DateTime.Now.Ticks - _startTicks) / 10000000.0;
        }

        /// <summary>
        ///   Returns FlightData and a time
        /// </summary>
        static Func<Tuple<FlightData,double>> BMSSource()
        {
            SharedMemory memoryArea1 = new SharedMemory("FalconSharedMemoryArea");
            SharedMemory memoryArea2 = new SharedMemory("FalconSharedMemoryArea2");

            memoryArea1.Open();
            memoryArea2.Open();

            return delegate() {
                FlightData data = (FlightData)memoryArea1.MarshalTo(typeof(FlightData));
                return Tuple.Create(data, Now());
            };
        }

        static Task _recordFlushOp = null;
        static void Drive(Func<Tuple<FlightData, double>> source, TextWriter recordWriter, Action<String> transmit, Action<Stats> reportStats)
        {
            // How long to sleep between polling, in ms
            int interval = 10;

            int history = 3;
            double[] ts = new double[history];
            Vector3D[] pos = new Vector3D[history];
            // Yaw, pitch, roll
            Vector3D[] seats = new Vector3D[history];
            Vector3D[] vAirframe = new Vector3D[history];
            Vector3D[] vSeats = new Vector3D[history];
            Vector3D[] fs = new Vector3D[history];
            long counter = 0;

            double deg20 = ToRadians(20);
            double deg6 = ToRadians(6);
            double G = 32.174F;

            Vector3D gravity = new Vector3D(0, 0, G);

            // Where the seat is relative to the center of rotation,
            // in feet, in aircraft coords.
            Vector3D seatPos = new Vector3D(10, 0, -3);

            // The G force on the various parts of the seat when at rest.
            double backNeutralG = sin(deg20);
            double seatNeutralG = cos(deg6);

            Vector3D blNormal = Transform(YPR(deg20, deg20, 0),
                                          new Vector3D(-1, 0, 0));
            Vector3D brNormal = Transform(YPR(-deg20, deg20, 0),
                                          new Vector3D(-1, 0, 0));
            Vector3D slNormal = Transform(YPR(0, deg6, deg20),
                                          new Vector3D(0, 0, 1));
            Vector3D srNormal = Transform(YPR(0, deg6, -deg20),
                                          new Vector3D(0, 0, 1));

            while (true)
            {
                long index = counter % history;

                var sourceData = source();
                FlightData data = sourceData.Item1;
                double t = sourceData.Item2;

                double x = data.x;
                double y = data.y;
                double z = data.z;
                double yaw = data.yaw;
                double pitch = data.pitch;
                double roll = data.roll;

                // Rotating index into the various history arrays
                int[] ago = new int[history];
                for (int n = 0; n < history; ++n)
                {
                    ago[n] = (int) ((index - n + history) % history);
                }
                int now = ago[0];

                ts[now] = t;
                pos[now] = new Vector3D(x, y, z);

                // If position hasn't changed, we've probably read the
                // same sampled data twice - skip updating, since it's
                // going to look like we've instantly gone to zero
                // speed.
                if (!pos[now].Equals(pos[ago[1]]))
                {
                
                    Matrix3D xform = YPR(yaw, pitch, roll);
                    seats[now] = Transform(xform, seatPos);

                    vAirframe[now] = new Vector3D(data.xDot, data.yDot, data.zDot);
                    //vAirframe[now] = Subtract(pos[now], pos[ago[1]]);
                    vSeats[now] = Scale(Subtract(seats[now], seats[ago[1]]), ts[now] - ts[ago[1]]);

                    Vector3D bln = Transform(xform, blNormal);
                    Vector3D brn = Transform(xform, brNormal);
                    Vector3D sln = Transform(xform, slNormal);
                    Vector3D srn = Transform(xform, srNormal);

                    // ft/sec/sec
                    double deltaT = ts[now] - ts[ago[1]];
                    Vector3D aAirframe = Scale(Subtract(vAirframe[now], vAirframe[ago[1]]), 1.0 / deltaT);
                    Vector3D aSeat = Scale(Subtract(vSeats[now], vSeats[ago[1]]), 1.0 / deltaT);
                    Vector3D acc = Add(aAirframe, aSeat);

                    // Turns out the position and velocity data coming
                    // from Falcon is pretty noisy. We smooth things out
                    // to help with this.
                    double smoothing = 0.9;
                    fs[now] = Add(Scale(fs[ago[1]], smoothing),
                                  Scale(Add(gravity, Scale(acc, -1.0)), 1.0 - smoothing));

                    Vector3D f = fs[now];

                    double bl = Dot(f, bln);
                    double br = Dot(f, brn);
                    double sl = Dot(f, sln);
                    double sr = Dot(f, srn);

                    double blG = bl / G;
                    double brG = br / G;
                    double slG = sl / G;
                    double srG = sr / G;

                    // TODO: Map the neutral (1G) position as .25
                    long commandBL = Position(blG, backNeutralG);
                    long commandBR = Position(brG, backNeutralG);
                    long commandSL = Position(slG, seatNeutralG);
                    long commandSR = Position(srG, seatNeutralG);

                    Stats stats;
                    stats.Forces.BL = bl;
                    stats.Forces.BR = br;
                    stats.Forces.SL = sl;
                    stats.Forces.SR = sr;
                    stats.G.BL = blG;
                    stats.G.BR = brG;
                    stats.G.SL = slG;
                    stats.G.SR = srG;
                    stats.Commands.BL = commandBL;
                    stats.Commands.BR = commandBR;
                    stats.Commands.SL = commandSL;
                    stats.Commands.SR = commandSR;
                    stats.Yaw = yaw;
                    stats.Pitch = pitch;
                    stats.Roll = roll;
                    stats.VAC = vAirframe[now];
                    stats.DVAC = aAirframe;
                    stats.DVSeat = aSeat;
                    stats.DVTot = f;
                    stats.T = ts[now];
                    stats.DeltaT = ts[now] - ts[ago[1]];

                    // We skip the first few frames until we have enough
                    // history to do calculation
                    if (counter > history)
                    {
                        reportStats(stats);
                        //ts[now] - ts[ago[1]], brG, blG, srG, slG);
                    
                        if (recordWriter != null)
                        {
                            if (_recordFlushOp != null)
                            {
                                _recordFlushOp.Wait();
                            }
                            recordWriter.WriteLine("{0}, {1}, {2}, {3}, {4}, {5}, {6}, {7}, {8}, {9}, {10}, {11}, {12}, {13}, {14}, {15}, {16}, {17}",
                                                   t,
                                                   data.x, data.y, data.z
                                                   data.xDot, data.yDot, data.zDot,
                                                   yaw, pitch, roll,
                                                   commandBL, commandBR,
                                                   commandSL, commandSR,
                                                   acc.X, acc.Y, acc.Z,
                                                   data.alpha);

                            _recordFlushOp = recordWriter.FlushAsync();
                        }

                        transmit(String.Format("M BL {0}", commandBL));
                        transmit(String.Format("M BR {0}", commandBR));
                        transmit(String.Format("M SL {0}", commandSL));
                        transmit(String.Format("M SR {0}", commandSR));
                    }
                    ++counter;
                }
                Sleep(interval);
            }
        }

        static Action<Stats> StatsReporter(StatsDialog dlg)
        {
            return (Action<Stats>) ((stats) =>
                                    {
                                        dlg.BeginInvoke((Action) (() => { dlg.Update(stats); }));
                                    });
        }

        static Action<Dictionary<string, double>> MakeConfigurator(Action<string> transmit)
        {
            return (Action<Dictionary<string, double>>) ((config) =>
                    {
                        StringBuilder sb = new StringBuilder();
                        foreach (var entry in config) {
                            foreach (var channel in new [] { "BL", "BR" })
                            {
                                sb.Append(String.Format("CONFIG {0} {1} {2}\n", channel, entry.Key, entry.Value));
                            }
                        }
                        transmit(sb.ToString());
                    });
                                                        
        }

        static void SerialTest(string port)
        {
            SerialPort serialPort = new SerialPort(port, 9600);
            serialPort.Open();
            serialPort.NewLine = "\n";
            serialPort.RtsEnable = true;
            serialPort.WriteLine("PING");
            Console.WriteLine("Awaiting response");
            Console.WriteLine(serialPort.ReadLine());
            Console.WriteLine("Serial test complete");
            serialPort.Close();
        }
        
        static void Main(String[] args)
        {
            TextWriter recordWriter = null;
            String port = null;
            String telemetryPath = null;
            bool serialTest = false;
            bool chopTest = false;
            int chopInterval = 10; // msec
            int chopStep = 100; // Units
            for (int i = 0; i < args.Length; ++i)
            {
                if (args[i] == "--record")
                {
                    ++i;
                    if ((i < args.Length) && !args[i].StartsWith("--"))
                    {
                        recordWriter = new StreamWriter(args[i]);
                    }
                    else
                    {
                        recordWriter = Console.Out;
                    }
                }
                else if (args[i] == "--port")
                {
                    port = args[++i];
                }
                else if (args[i] == "--telemetry")
                {
                    telemetryPath = args[++i];
                }
                else if (args[i] == "--serialtest")
                {
                    serialTest = true;
                }
                else if (args[i] == "--choptest")
                {
                    chopTest = true;
                    chopInterval = Int32.Parse(args[++i]);
                    chopStep = Int32.Parse(args[++i]);
                }
                else
                {
                    Console.WriteLine("Unrecognized command line option: ", args[i]);
                    return;
                }
            }

            if (serialTest)
            {
                SerialTest(port);
                return;
            }

            Action<string> write;
            Func<string> read;
            if (port == null)
            {
                write = (s) => Console.WriteLine(s);
                read = () => {
                    Thread.Sleep(Int32.MaxValue);
                    return "";
                };
            }
            else
            {
                SerialPort serialPort = new SerialPort(port, 9600);
                serialPort.RtsEnable = true;
                serialPort.Open();
                write = (s) => serialPort.WriteLine(s);
                read = () => serialPort.ReadLine();
            }

            Action<string> reportTelemetry;
            if (telemetryPath == null)
            {
                reportTelemetry = (s) => Console.WriteLine(s);
            }
            else
            {
                StreamWriter file = new StreamWriter(telemetryPath);
                reportTelemetry = (s) => file.WriteLine(s);
            }
            
            StatsDialog stats = new StatsDialog();
            ConfigDialog config = new ConfigDialog(MakeConfigurator(write));

            var writeQueue = new BlockingCollection<string>();
            var readQueue = new BlockingCollection<string>();
            Thread writeWorker = new Thread(delegate()
                                            {
                                                while (true)
                                                {
                                                    write(writeQueue.Take());
                                                }
                                            });
            Thread readWorker = new Thread(delegate()
                                           {
                                               while(true)
                                               {
                                                   readQueue.Add(read());
                                               }
                                           });
            Thread worker = new Thread(delegate()
                                       {
                                           Drive(BMSSource(),
                                                 recordWriter,
                                                 (s) => writeQueue.Add(s),
                                                 StatsReporter(stats));
                                       });

            Thread telemetryWorker =
                new Thread(delegate()
                           {
                               while (true)
                               {
                                   reportTelemetry(readQueue.Take());
                               }
                           });

            telemetryWorker.Start();
            writeWorker.Start();
            readWorker.Start();

            if (chopTest)
            {
                writeQueue.Add("M BL 9000");
                writeQueue.Add("M BL 9000");
                Thread.Sleep(1000);

                for (int pos = 9000; pos > 1000; pos -= chopStep)
                {
                    writeQueue.Add(String.Format("M BL {0}", pos));
                    writeQueue.Add(String.Format("M BR {0}", pos));
                    Thread.Sleep(chopInterval);
                }
            }
            else {
                worker.Start();
                config.Show();
                System.Windows.Forms.Application.Run(stats);
            }

        }
    }
}
