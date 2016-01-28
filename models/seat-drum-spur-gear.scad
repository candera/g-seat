include <MCAD/involute_gears.scad>

// This looks like it will produce a 6" gear that will mesh with the
// worm shaft on the motor I ordered
gear_diameter = 6.5 * 25.4;
gear_radius = gear_diameter / 2;
gear_thickness = 5; // How thick the center part is
hub_thickness = 8;
bore_diameter = 25.4 * 3 / 8;
hub_diameter = bore_diameter * 2;
rim_thickness = 8;
rim_width = 5;

// Screw holes in gear 2
screw_holes = 7;
screw_hole_diameter = 4;
screw_hole_location = 0.75; // Distance from middle as proportion of radius
screw_hole_phase = 0.5;

difference () {
  union () {
    gear(number_of_teeth=round(gear_diameter * .37),
         diametral_pitch=0.38,
         //pressure_angle=38,  // Not sure if this is right - need motor
         circles=7,
         gear_thickness=gear_thickness,
         hub_thickness=hub_thickness,
         hub_diameter=hub_diameter,
         bore_diameter=bore_diameter,
         rim_width=rim_width,
         rim_thickness=rim_thickness
         );

    // Screw hole surrounds
    for ( i = [1:screw_holes] ) {
      rotate([0, 0, (i + screw_hole_phase) * (360/screw_holes)]) {
        translate ([gear_radius * screw_hole_location, 0, 0]) {
          cylinder(h=hub_thickness,
                   r=screw_hole_diameter);
        }
      }
    }
  }

  // Screw holes
  for ( i = [1:screw_holes] ) {
    rotate([0, 0, (i + screw_hole_phase) * (360/screw_holes)]) {
      translate ([gear_radius * screw_hole_location, 0, 0]) {
        cylinder(h=hub_thickness+0.1,
                 r=screw_hole_diameter/2);

      }
    }
  }

}
