$fa = 1;
$fs = 0.15;

thick         = 2; 
vent_width    = 1.5;   

box_x_adj = 0.5;
box_y_adj = 0.2;
box_z_adj = 0;

//Box
difference(){
    union(){
        difference(){
            difference(){
                translate([-2,-2,0])
                roundedcube([54+box_x_adj,46+box_y_adj,27]);
                cube([50+box_x_adj,42+box_y_adj,25]);
            }
            translate([-2,(box_y_adj/2),0])
            cube([2,42,9]);
            translate([-2,4.5+(box_y_adj/2),9])
            cube([2,9.4,8]);
        }

        translate([-2,18.2+(box_y_adj/2),7])
        cube([2,13.6,2]);
    }

    //vents
    for(i=[0:thick+1:15]){
        translate([i,42,17])
        cube([vent_width,2,10]);
        
        translate([i,-2,17])
        cube([vent_width,2,10]);
        
        translate([24+i,12+(box_y_adj/2),25])
        cube([vent_width,18,3]);
    }

}

module roundedcube(size = [1, 1, 1], center = false, radius = 0.5, apply_to = "all") {
	// If single value, convert to [x, y, z] vector
	size = (size[0] == undef) ? [size, size, size] : size;

	translate_min = radius;
	translate_xmax = size[0] - radius;
	translate_ymax = size[1] - radius;
	translate_zmax = size[2] - radius;

	diameter = radius * 2;

	obj_translate = (center == false) ?
		[0, 0, 0] : [
			-(size[0] / 2),
			-(size[1] / 2),
			-(size[2] / 2)
		];

	translate(v = obj_translate) {
		hull() {
			for (translate_x = [translate_min, translate_xmax]) {
				x_at = (translate_x == translate_min) ? "min" : "max";
				for (translate_y = [translate_min, translate_ymax]) {
					y_at = (translate_y == translate_min) ? "min" : "max";
					for (translate_z = [translate_min, translate_zmax]) {
						z_at = (translate_z == translate_min) ? "min" : "max";

						translate(v = [translate_x, translate_y, translate_z])
						if (
							(apply_to == "all") ||
							(apply_to == "xmin" && x_at == "min") || (apply_to == "xmax" && x_at == "max") ||
							(apply_to == "ymin" && y_at == "min") || (apply_to == "ymax" && y_at == "max") ||
							(apply_to == "zmin" && z_at == "min") || (apply_to == "zmax" && z_at == "max")
						) {
							sphere(r = radius);
						} else {
							rotate = 
								(apply_to == "xmin" || apply_to == "xmax" || apply_to == "x") ? [0, 90, 0] : (
								(apply_to == "ymin" || apply_to == "ymax" || apply_to == "y") ? [90, 90, 0] :
								[0, 0, 0]
							);
							rotate(a = rotate)
							cylinder(h = diameter, r = radius, center = true);
						}
					}
				}
			}
		}
	}
}