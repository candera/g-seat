using System;
using System.Drawing;
using System.Windows.Forms;

namespace driver {
    public static class Functions
    {
        public static void PopulateVisComponents(Form form, VisComponent[] visComponents, int spacing)
        {
            for (int index = 0; index < visComponents.Length; ++index)
            {
                VisComponent c = visComponents[index];
                int y = index * spacing + 10;

                int x = 10;
                int w = 170;
                c.Label = new Label();
                c.Label.Text = c.Name;
                c.Label.Location = new System.Drawing.Point(x, y);
                c.Label.Size = new System.Drawing.Size(w, 28);

                x += w + 10;
                w = 100;
                c.ValueLabel = new Label();
                c.ValueLabel.Text = String.Format("{0:F2}", c.Value);
                c.ValueLabel.Location = new Point(x, y);
                c.ValueLabel.Size = new Size(w, 28);

                x += w + 10;
                w = 300;
                c.TrackBar = new TrackBar();
                c.TrackBar.Location = new System.Drawing.Point(x, y);
                c.TrackBar.Size = new System.Drawing.Size(w, 28);
                c.TrackBar.Minimum = 0;
                c.TrackBar.Maximum = c.Ticks;
                c.TrackBar.TickFrequency = c.Ticks/10;
                c.TrackBar.Value = ComputeTicks(c, c.Value);

                c.TrackBar.ValueChanged += delegate(object sender, EventArgs e)
                    {
                        c.ValueLabel.Text = String.Format("{0:F2}", ComputeValue(c));
                    };

            }

            foreach (var c in visComponents) {
                form.Controls.Add(c.Label);
                form.Controls.Add(c.TrackBar);
                form.Controls.Add(c.ValueLabel);
            }


        }

        public static double ComputeValue(VisComponent c)
        {
            return c.Min + ((c.Max - c.Min) * c.TrackBar.Value / (double) c.Ticks);
        }

        public static int ComputeTicks(VisComponent c, double val)
        {
            int v = (int) (c.Ticks * ((val - c.Min) / (c.Max - c.Min)));

            if (v < 0)
            {
                v = 0;
            }

            if (v > c.Ticks)
            {
                v = c.Ticks;
            }

            return v;
        }

    }
}
