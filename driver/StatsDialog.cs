using System;
using System.Drawing;
using System.Collections;
using System.ComponentModel;
using System.Windows.Forms;

namespace driver
{
    /// <summary>
    /// Summary description for StatsDialog.
    /// </summary>
    public class StatsDialog : System.Windows.Forms.Form
    {
        public class VisComponent
        {
            public String Name;
            public Label Label;
            public TrackBar TrackBar;
            public double Min;
            public double Max;
        }

        private int ticks = 1000;

        private VisComponent[] visComponents = new[] {
            new VisComponent { Name = "BL", Min = -3, Max = 9 },
            new VisComponent { Name = "BR", Min = -3, Max = 9 },
            new VisComponent { Name = "SL", Min = -3, Max = 9 },
            new VisComponent { Name = "SR", Min = -3, Max = 9 },
            new VisComponent { Name = "DT", Min = 0.09, Max = 0.11 },
            new VisComponent { Name = "Yaw", Min = -3.2, Max = 3.2 },
            new VisComponent { Name = "Pitch", Min = -3.2, Max = 3.2 },
            new VisComponent { Name = "Roll", Min = -3.2, Max = 3.2 },
            new VisComponent { Name = "VACX", Min = -1000, Max = 1000 },
            new VisComponent { Name = "VACY", Min = -1000, Max = 1000 },
            new VisComponent { Name = "VACZ", Min = -1000, Max = 1000 },
            new VisComponent { Name = "DVACX", Min = -20, Max = 20 },
            new VisComponent { Name = "DVACY", Min = -20, Max = 20 },
            new VisComponent { Name = "DVACZ", Min = -20, Max = 20 },
            new VisComponent { Name = "DVSeatX", Min = -20, Max = 20 },
            new VisComponent { Name = "DVSeatY", Min = -20, Max = 20 },
            new VisComponent { Name = "DVSeatZ", Min = -20, Max = 20 },
            new VisComponent { Name = "DVTotX", Min = -20, Max = 20 },
            new VisComponent { Name = "DVTotY", Min = -20, Max = 20 },
            new VisComponent { Name = "DVTotZ", Min = -20, Max = 20 },
            new VisComponent { Name = "CmdBL", Min = 0, Max = 10000},
            new VisComponent { Name = "CmdBR", Min = 0, Max = 10000},
            new VisComponent { Name = "CmdSL", Min = 0, Max = 10000},
            new VisComponent { Name = "CmdSR", Min = 0, Max = 10000},
        };

        private System.Windows.Forms.ErrorProvider timeErrorProvider;
        private IContainer components;

        public StatsDialog()
        {
            //
            // Required for Windows Form Designer support
            //
            InitializeComponent();
        }

        /// <summary>
        /// Clean up any resources being used.
        /// </summary>
        protected override void Dispose(bool disposing)
        {
            if (disposing)
            {
                if (components != null)
                {
                    components.Dispose();
                }
            }
            base.Dispose(disposing);
        }

        #region Windows Form Designer generated code

        /// <summary>

        /// Required method for Designer support - do not modify

        /// the contents of this method with the code editor.

        /// </summary>

        private void InitializeComponent()
        {
            this.components = new System.ComponentModel.Container();

            this.timeErrorProvider = new System.Windows.Forms.ErrorProvider(this.components);
            ((System.ComponentModel.ISupportInitialize)(this.timeErrorProvider)).BeginInit();
            this.SuspendLayout();

            for (int index = 0; index < visComponents.Length; ++index)
            {
                VisComponent c = visComponents[index];
                int spacing = 60;
                int y = index * spacing + 10;

                c.Label = new Label();
                c.Label.Text = c.Name;
                c.Label.Location = new System.Drawing.Point(10, y);
                c.Label.Size = new System.Drawing.Size(150, 28);

                c.TrackBar = new TrackBar();
                c.TrackBar.Location = new System.Drawing.Point(165, y);
                c.TrackBar.Size = new System.Drawing.Size(300, 28);
                c.TrackBar.Minimum = 0;
                c.TrackBar.Maximum = ticks;
                c.TrackBar.TickFrequency = ticks/10;
            }

            //
            // StatsDialog
            //
            this.AutoScaleBaseSize = new System.Drawing.Size(15, 34);
            this.ClientSize = new System.Drawing.Size(500, 500);

            this.Font = new System.Drawing.Font("Microsoft Sans Serif", 18F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.MaximizeBox = false;
            this.MaximumSize = new System.Drawing.Size(1250, 500);
            this.MinimizeBox = false;
            this.MinimumSize = new System.Drawing.Size(0, visComponents.Length * 60 + 20);
            this.Name = "StatsDialog";
            this.Text = "StatsDialog";
            ((System.ComponentModel.ISupportInitialize)(this.timeErrorProvider)).EndInit();

            foreach (var c in visComponents) {
                this.Controls.Add(c.Label);
                this.Controls.Add(c.TrackBar);
            }

            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private VisComponent LookupVisComponent(String name)
        {
            foreach (var c in visComponents)
            {
                if (c.Name == name)
                {
                    return c;
                }
            }
            return null;
        }

        private void UpdateSingle(String name, double val)
        {
            VisComponent c = LookupVisComponent(name);
            int v = (int) (ticks * ((val - c.Min) / (c.Max - c.Min)));

            if (v < 0)
            {
                v = 0;
            }

            if (v > ticks)
            {
                v = ticks;
            }

            c.TrackBar.Value = v;
        }

        private void UpdateSingle(String name, long val)
        {
            UpdateSingle(name, (double) val);
        }

        public void Update(Stats stats)
        {
            UpdateSingle("BR", stats.G.BR);
            UpdateSingle("BL", stats.G.BL);
            UpdateSingle("SR", stats.G.SR);
            UpdateSingle("SL", stats.G.SL);
            UpdateSingle("DT", stats.DeltaT);
            UpdateSingle("Yaw", stats.Yaw);
            UpdateSingle("Pitch", stats.Pitch);
            UpdateSingle("Roll", stats.Roll);
            UpdateSingle("VACX", stats.VAC.X);
            UpdateSingle("VACY", stats.VAC.Y);
            UpdateSingle("VACZ", stats.VAC.Z);
            UpdateSingle("DVACX", stats.DVAC.X);
            UpdateSingle("DVACY", stats.DVAC.Y);
            UpdateSingle("DVACZ", stats.DVAC.Z);
            UpdateSingle("DVSeatX", stats.DVSeat.X);
            UpdateSingle("DVSeatY", stats.DVSeat.Y);
            UpdateSingle("DVSeatZ", stats.DVSeat.Z);
            UpdateSingle("DVTotX", stats.DVTot.X);
            UpdateSingle("DVTotY", stats.DVTot.Y);
            UpdateSingle("DVTotZ", stats.DVTot.Z);
            UpdateSingle("CmdBR", stats.Commands.BR);
            UpdateSingle("CmdBL", stats.Commands.BL);
            UpdateSingle("CmdSR", stats.Commands.SR);
            UpdateSingle("CmdSL", stats.Commands.SL);
        }

    }
}
