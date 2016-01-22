using System;
using System.Drawing;
using System.Collections;
using System.Collections.Generic;
using System.ComponentModel;
using System.Windows.Forms;

namespace driver
{
    /// <summary>
    /// Summary description for ConfigDialog.
    /// </summary>
    public class ConfigDialog : System.Windows.Forms.Form
    {
        private VisComponent[] visComponents = new[] {
            new VisComponent { Name = "KP", Min = 0, Max = 10, Value = 8.0 },
            new VisComponent { Name = "GAP", Min = 0, Max = 2500, Value = 500 },
            new VisComponent { Name = "RES", Min = -3, Max = 9, Value = 0 },
            new VisComponent { Name = "KVTARGET", Min = 0, Max = 10, Value = 0.5 },
            new VisComponent { Name = "KVBOOST", Min = 0, Max = 10, Value = 0 },
            new VisComponent { Name = "GOALSMOOTHING", Min = 0, Max = 1, Value = 0.1 },
            new VisComponent { Name = "DRIVESMOOTHING", Min = 0, Max = 1, Value = 0.1 },
        };

        private Button apply;
        private Action<Dictionary<string, double>> configure;

        private System.Windows.Forms.ErrorProvider timeErrorProvider;
        private IContainer components;

        public ConfigDialog(Action<Dictionary<string, double>> configure)
        {
            //
            // Required for Windows Form Designer support
            //
            InitializeComponent();

            this.configure = configure;
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

            int spacing = 60;
            Functions.PopulateVisComponents(this, visComponents, spacing);

            this.apply = new Button();
            apply.Text = "Apply";
            apply.Click += ApplyClick;
            apply.Location = new Point(10, 10 + ((visComponents.Length + 1) * spacing));
            apply.Size = new Size(150, 35);
            
            //
            // ConfigDialog
            //
            this.AutoScaleBaseSize = new System.Drawing.Size(15, 34);
            this.ClientSize = new System.Drawing.Size(800, 800);

            this.Font = new System.Drawing.Font("Microsoft Sans Serif", 18F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.MaximizeBox = false;
            this.MaximumSize = new System.Drawing.Size(1250, 800);
            this.MinimizeBox = false;
            this.MinimumSize = new System.Drawing.Size(0, visComponents.Length * spacing + 20);
            this.Name = "ConfigDialog";
            this.Text = "ConfigDialog";
            ((System.ComponentModel.ISupportInitialize)(this.timeErrorProvider)).EndInit();

            this.Controls.Add(apply);

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

        private VisComponent LookupVisComponent(TrackBar trackBar)
        {
            foreach (var c in visComponents)
            {
                if (c.TrackBar == trackBar)
                {
                    return c;
                }
            }
            return null;
        }

        void ApplyClick(object sender, EventArgs e)
        {
            Dictionary<string, double> config = new Dictionary<string, double>();

            foreach (var c in visComponents)
            {
                config.Add(c.Name, Functions.ComputeValue(c));
            }

            configure(config);
        }


    }
}
