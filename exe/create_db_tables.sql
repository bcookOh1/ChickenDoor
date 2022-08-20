
drop table door_state;

create table door_state (
  'id' INTEGER PRIMARY KEY AUTOINCREMENT,
  'timestamp' text not null,
  'state' int not null,
  'light' text not null,
  'pi_temp' text not null
  'decision' text not null
);

insert into door_state (timestamp, state, light, pi_temp)
values
 ('2020-07-17 13:10:00',1,'500.0','30.5','Sunrise_W_Offset'),
 ('2020-07-17 13:12:00',2,'500.0','30.5','Sunrise_W_Offset'),
 ('2020-07-17 13:30:00',1,'500.0','30.5','Sunrise_W_Offset'),
 ('2020-07-17 13:32:00',2,'500.0','30.5','Sunrise_W_Offset'),
 ('2020-07-17 13:50:00',1,'500.0','30.5','Sunrise_W_Offset'),
 ('2020-07-17 14:00:00',2,'500.0','30.5','Sunrise_W_Offset'),
 ('2020-07-17 14:10:00',1,'500.0','30.5','Sunrise_W_Offset'),
 ('2020-07-17 14:20:00',2,'500.0','30.5','Sunrise_W_Offset');


select *
from door_state
order by id desc;

drop table readings;

create table readings (
  'id' INTEGER PRIMARY KEY AUTOINCREMENT,
  'timestamp' text not null,
  'temperature' text not null,
  'temperature_units' text not null,
  'humidity' text not null,
  'humidity_units' text not null,
  'light' text not null,
  'light_units' text not null
);

insert into readings (timestamp, temperature, temperature_units, humidity, humidity_units, light, light_units)
values
 ('2021-02-05 13:10:00','60.0','degF','30.0','%','500.0','lx'),
 ('2021-02-05 13:10:02','60.1','degF','30.1','%','500.0','lx'),
 ('2021-02-05 13:10:04','60.2','degF','30.2','%','500.0','lx'),
 ('2021-02-05 13:10:06','60.3','degF','30.3','%','500.0','lx'),
 ('2021-02-05 13:10:08','60.4','degF','30.4','%','500.0','lx'),
 ('2021-02-05 13:10:10','60.5','degF','30.5','%','500.0','lx'),
 ('2021-02-05 13:10:12','60.6','degF','30.6','%','500.0','lx'),
 ('2021-02-05 13:10:14','60.7','degF','30.7','%','500.0','lx');

select *
from readings
order by id desc;


drop table sun_data;

create table sun_data (
  'id' INTEGER PRIMARY KEY AUTOINCREMENT,
  'timestamp' text not null,
  'sunrise' text not null,
  'sunset' text not null
);

insert into sun_data (timestamp, sunrise, sunset)
values
 ('2021-02-05 13:10:00','06:50:00','20:01:00'),
 ('2021-02-05 13:10:02','06:50:00','20:01:00'),
 ('2021-02-05 13:10:04','06:50:00','20:01:00'),
 ('2021-02-05 13:10:06','06:50:00','20:01:00'),
 ('2021-02-05 13:10:08','06:50:00','20:01:00'),
 ('2021-02-05 13:10:10','06:50:00','20:01:00'),
 ('2021-02-05 13:10:12','06:50:00','20:01:00'),
 ('2021-02-05 13:10:14','06:50:00','20:01:00');

select *
from sun_data
order by id desc;