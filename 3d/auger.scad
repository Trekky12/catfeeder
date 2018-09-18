use <CustomizablePrintableAuger-2.scad>


create_auger();

drivemount();

*servo();

tubemount();

servoshim();

*%pvctube();


module pvctube() {
    difference() {
        cylinder(h=145, r=40/2, $fn=48);
        // innerer Zylinder ist 2mm höher und daher um 1mm nach unten verschoben
        translate([0,0,-1]) cylinder(h=150, r=35/2, $fn=48);
    }
}


module create_auger() {
    difference() {
        // Spirale andersrum
        color([0,1,0]) translate([0,0,145]) mirror([0,0,1]) auger(r1=4.5, r2=34/2, h=145, turns=5, multiStart=1, flightThickness = 2, overhangAngle=20, supportThickness=0);
        // Mittleres Loch nochmal abziehen
        translate([0,0,-1]) cylinder(h=2.5, r=8/2, $fn=20);
    }
}

module drivemount() {
     translate([0,0,-2]) difference(){
        cylinder(h=5, r=34/2, $fn=48);
        for (angle = [ 0 ,60, 120, 180, 240, 300]) translate([0,0,-1]) rotate([0,0,angle]) {
            
            translate([13.5,0,0]) cylinder(h=8, r=2/2, $fn=12); 
            translate([10.5,0,0]) cylinder(h=8, r=2/2, $fn=12);
            translate([7.5,0,0]) cylinder(h=8, r=2/2, $fn=12);
            
            linear_extrude(height=3) translate([0,-7/2,0]) polygon([[0,0],[0,7],[7.5,7],[7.5+10, (7-4)/2+4], [7.5+10,(7-4)/2], [7.5,0]]);
        }
        // Loch in der Mitte
        translate([0,0,-1]) cylinder(h=4.5, r=8/2, $fn=20);
    }
    
    // Helper zur Abmessung der Lochabstände
    *color([0,1,0]) translate([0,0,-1])  cylinder(h=5, r=28.3/2, $fn=60);
    *color([0,1,0]) translate([0,0,-1])  cylinder(h=5, r=22.3/2, $fn=60);
    *color([0,1,0]) translate([0,0,-1])  cylinder(h=5, r=16.3/2, $fn=60);
}



module servo() {
    
    color([0,0.7,.3])
    translate([(-6.8/2)-14.5,-20/2,-17]) union(){
        cube([55.4,20,3]);
        color([1,0,0]) translate([14.8/2,0,3]) cube([40.6,20,11]);
        
        *color([1,0,0]) translate([6.8/2,10/2,1]) cube([48.6,10,3]);
            
        // Löcher
        translate([6.8/2,5,0]) cylinder(h=10, r=4.5/2);
        translate([6.8/2,5+10,0]) cylinder(h=10, r=4.5/2);
        translate([6.8/2+48.6,5,0]) cylinder(h=10, r=4.5/2);
        translate([6.8/2+48.6,5+10,0]) cylinder(h=10, r=4.5/2);
            
        // Motor
        translate([6.8/2+14.5,10,0]) cylinder(h=18, r=3, $fn=24);
        
        
        // Mittelsteg
        translate([3,20/2-1.25,0]) cube([55.4-2*3,2.5,3+2]);
    }
}

module tubemount() {
    
    color([1,0,0])
    translate([0,0,-9]) difference() {
        union() {
            difference() {
                union() {
                    // Innerer Zylinder (Außenkante)
                    cylinder(h=28, r=40/2, $fn=40);
                    // Äußerer Zylinder
                    translate([0,0,-4]) cylinder(h=11, r=50/2, $fn=48);
                    // Ablage vorne
                    translate([15,-26/2,-4]) cube([25.5,26,4]);

                }
                // Innerer Zylinder (Innenkante)
                translate([0,0,-10]) cylinder(h=50, r=36/2, $fn=48);
                
                // Zylinder Ausschnitt
                translate([-14.5+(48.6-40.6)/2+1,-(20+2)/2,-6]) cube([40.6,20+2,14]);
                
            }
            // Ablage hinten
            translate([-20,-28/2,-4]) cube([8.5,28,4]);   
        }
        // Löcher
         translate([(-6.8/2)-14.5,-20/2,-5]) union(){
            translate([6.8/2,5,0]) cylinder(h=10, r=4.5/2);
            translate([6.8/2,5+10,0]) cylinder(h=10, r=4.5/2);
            translate([6.8/2+48.6,5,0]) cylinder(h=10, r=4.5/2);
            translate([6.8/2+48.6,5+10,0]) cylinder(h=10, r=4.5/2);
             
            // Mittelsteg
            translate([3,20/2-1.25,0]) cube([55.4-2*3,2.5,4]);
        }
        
    }
    
}


module servoshim() {
    color([.7,.7,0])
    translate([0,0,-1-3-20]) difference(){
        difference(){
            translate([-20,-26/2,0]) cube([20+25.5+15,26,4]);  
            translate([-14.5+(48.6-40.6)/2-1,-(20+2)/2,-1]) cube([40.6+2,20+2,8]); 
        }
        translate([(-6.8/2)-14.5,-20/2,-4]) union(){
            translate([6.8/2,5,0]) cylinder(h=10, r=4.5/2);
            translate([6.8/2,5+10,0]) cylinder(h=10, r=4.5/2);
            translate([6.8/2+48.6,5,0]) cylinder(h=10, r=4.5/2);
            translate([6.8/2+48.6,5+10,0]) cylinder(h=10, r=4.5/2);
        }
    }
}

