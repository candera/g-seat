using System;
using System.Windows.Forms;

namespace driver {
    public class VisComponent
    {
        public String Name;
        public Label Label;
        public Label ValueLabel;
        public TrackBar TrackBar;
        public double Min;
        public double Max;
        public double Value;
        public int Ticks = 1000;
    }
}
