-- system clear;

use twitter4;

set profiling=1;

select status, count(*) from users group by status order by status limit 5;

select count(*), count(distinct uid), count(distinct youtube_video_id) from tweets;

select * from meta; select check_out_at, check_out_ip, sec_until_retry from credentials order by check_out_at limit 5;

show profiles;
