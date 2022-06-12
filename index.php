
<!DOCTYPE html> 
<html> 
   <head>
      <link href="coop.css" rel="stylesheet">
      <title>Garage Door Monitor</title>
   </head>
   <body>
      <?php 

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
                  return "NoChange";
                  break;  
               default:  
                  return "None";
                  break;  
            }
         }
     

         class GarageDB extends SQLite3 {
            function __construct() {
               $this->open('/home/bjc/coop/exe/coop.db');
            }
         }

         $db = new GarageDB();
         $db->exec('begin');
         $result = $db->query('select * from door_state order by id desc limit 11');

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
         echo "<img src='".$filename."' width='600'/>";

      ?> 
   </body> 
</html>
