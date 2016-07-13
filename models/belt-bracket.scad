belt_width = 50;
belt_thickness = 1;
belt_clearance = belt_thickness * 2.5;
plywood_thickness = 18.2;
end_cap_thickness = 4;
guide_radius = 13;
retainer_gap = 5;
screw_hole_diameter = 4;
dowel_diameter = 8;
dowel_length = 30;

end_cap_height = guide_radius+belt_clearance+end_cap_thickness;

difference () {
  union () {
    // Belt guide
    translate([end_cap_thickness,0,0]) {
      rotate ([0,90,0]) {
        cylinder($fn=96,r=guide_radius, h=belt_width/2);
      }
    }
    // End cap
    translate([0,-guide_radius,-end_cap_height]) {
      cube([end_cap_thickness, guide_radius*2, 2*end_cap_height]);
    }
    // Mounting plate
    translate([end_cap_thickness,-guide_radius,-end_cap_height]) {
      cube([belt_width/2, guide_radius*2, end_cap_height]);
    }
    // Retainer plate
    translate([end_cap_thickness,-guide_radius,end_cap_height-end_cap_thickness]) {
      cube([belt_width/2 - retainer_gap/2, guide_radius*2,end_cap_thickness]);
    }      
  }
  // Slot for the plywood
  translate([-0.1,-plywood_thickness/2,-end_cap_height-0.1]) {
    cube([end_cap_thickness+0.2+(belt_width/2),plywood_thickness,end_cap_height+0.1]);
  }
  /* // Screw hole */
  /* translate([(end_cap_thickness+plywood_thickness)/2, */
  /*            -(plywood_thickness-end_cap_thickness)/2, */
  /*            -plywood_thickness/2]) { */
  /*   rotate([90,0,0]) { */
  /*     cylinder($fn=24,r=screw_hole_diameter/2, h = end_cap_thickness + 0.2); */
  /*   } */
  /* } */
  /* // Dowel hole */
  /* translate([end_cap_thickness+(belt_width/2)-(dowel_length/2)+0.1,0,guide_radius/2]) { */
  /*   rotate([0,90,0]) { */
  /*     cylinder($fn=24,r=dowel_diameter/2,h=dowel_length/2+0.2); */
  /*   } */
  /* } */
}
