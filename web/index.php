
<!DOCTYPE html> 
<html> 
   <head>
      <link href="coop.css" rel="stylesheet">
      <title>Laura and Jess' Smart Chicken Coop</title>
   </head>
   <body>

      <?php 

         error_reporting(E_ERROR | E_WARNING | E_PARSE);

         function WriteMode(string $mode) { 

            $coop_ipc_path = "/home/bjc/coop/exe/user_input.txt";
            // echo $coop_ipc_path;

            $file = fopen($coop_ipc_path, "w");
            if($file == false) {
               echo "open failed ";
               return;
	         } // end if 
	
            fwrite($file, $mode);
            fclose($file);

            // original is 0644 then 0646 adds write perm to 'other' 
            chmod($coop_ipc_path, 0646);
 
         } // end WriteMode


         function PrintState($stateValue){

            switch($stateValue){
               case 1:  
                  return "Opened";
                  break;  
               case 2:  
                  return "MovingToClose";
                  break;  
               case 3:  
                  return "Closed";
                  break;  
               case 4:  
                  return "MovingToOpen";
                  break;  
               case 5: 
                  return "Obstructed";
                  break;  
               default:  
                  return "NoChange";
                  break;  
            }
         } // end PrintState
     

         class GarageDB extends SQLite3 {
            function __construct() {
               $this->open('/home/bjc/coop/exe/coop.db');
            }
         }

         $db = new GarageDB();
         $db->exec('begin');
         $result = $db->query('select * from door_state order by id desc limit 5');

         $first = 1;
         $rec_time = "None";
         $state = "None";
         $boardTemp = "None";
         $lightLevel = "None";

         while ($row = $result->fetchArray()) {

            $rec_time = $row['timestamp'];
            $stateStr = PrintState($row['state']);
            $tempC = $row['pi_temp'];
            $lightLevel = $row['light'];

            $t=substr($rec_time,11,strlen($rec_time));
            $y=substr($rec_time,0,4);
            $md=substr($rec_time,5,5);
            $d=$md . "-" . $y;
            $td = $t . " " . $d;

            // make a temp string like:  "20.6C,(69.2F)"
            $temp_str = sprintf("%.1fC (%.1fF)",(float)$tempC,($tempC*(9/5))+32);

            if($first == 1){

               echo "<p class=\"current\"> The chicken door is <span class=\"current\">$stateStr</span>, $td</p>";
               $first = 0;

               echo "<table id=\"history\">
                        <caption style=\"text-align:left\" >The last 10 Open and Closings: </caption>
                        <tr>
                           <th>Time and Date</th>
                           <th>State</th>
                           <th>Light Level</th>
                           <th>Pi Temperature</th>
                        </tr> ";
            }
            else {

               echo "<tr>
                        <td>$td</td>
                        <td>$stateStr</td>
                        <td>$lightLevel</td>
                        <td>$temp_str</td>
                    </tr>";
            }
         }
         echo "</table>"; 

         // get the ambient temperature, humidity, and light_level read from the i2c sensors
         $result = $db->query('select * from readings order by id desc limit 1');
         $row = $result->fetchArray();
         $timestamp = $row['timestamp'];
         $tempDegF = $row['temperature'];
         $humidity = $row['humidity'];
         $light = $row['light'];

         $db->exec('end');
         $db->close();

         // make a display string from the temp and humidity data 
         $tempHumid = sprintf("Timestamp: %s, Temperature: %sdegF, humidity: %s%%, light: %s(lx)", $timestamp, $tempDegF, $humidity, $light);
         echo "<p class=\"current\"> $tempHumid";
         
         // make the chart file
         exec("./gnup");

         $filename = "chart.png";
         echo "<p class=\"current\"> the chart </p>";
         echo "<img src=$filename width='600'/>";


      ?> 

      <form method="post">
        <input type="submit" name="button1" value="Manual UP"/>
        <input type="submit" name="button2" value="Manual Down"/>
        <input type="submit" name="button3" value="Auto Mode"/>
        <input type="submit" name="button3" value="Take Picture"/>


      	<?php 
        
            if(isset($_POST['button1'])) {
               WriteMode("u\n");
               echo  "up";
            }
            else if(isset($_POST['button2'])) {
               WriteMode("d\n");
               echo  "down";
            }
            else if(isset($_POST['button3'])) {
               WriteMode("a\n");
               echo  "auto";
            }
            else if(isset($_POST['button4'])) {
               WriteMode("c\n");
               echo  "take picture";
            }
        
        ?>

     </form>

   </body> 
</html>
