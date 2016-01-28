holes = 7;
hole_diameter = 28;
hole_location = 0.3;
pulley_diameter = 6 * 25.4;
pulley_radius = pulley_diameter / 2;
pulley_thickness = 7;
shaft_diameter = 0.1 + 3 / 8 * 25.4;
shaft_radius = shaft_diameter / 2;
belt_diameter = 4;
belt_radius = belt_diameter / 2;

difference () {
  cylinder($fn=200,h=pulley_thickness,r1=pulley_radius,r2=pulley_radius);
  translate ([0, 0, -0.05]) {
    for (i = [1:holes])
      rotate([0, 0, i * (360/holes)])
        translate ([pulley_diameter * hole_location, 0, 0])
        {
          cylinder(h=pulley_thickness + 0.1, r1=hole_diameter/2, r2=hole_diameter/2);
        }

    cylinder(h = pulley_thickness + 0.1, r1 = shaft_radius, r2 = shaft_radius);
  }

  translate ([0,0,pulley_thickness / 2]) {
    rotate_extrude ($fn=200) {
      translate ([pulley_radius + belt_radius/2, 0, 0])
        circle(r = belt_radius);
    }
  }
  
  
}
