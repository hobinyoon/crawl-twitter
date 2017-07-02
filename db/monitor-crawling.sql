-- system clear;

use twitter4;

set profiling=1;

select status, count(*) from users where status IN ('U', 'UC', 'UP') group by status order by status limit 5;
-- select status, count(*) from users group by status order by status;

-- select count(*), count(distinct uid), count(distinct youtube_video_id) from tweets;
select count(*) as num_youtube_reqs from tweets;

-- select * from meta;

select check_out_ip, max(check_out_at), min(check_out_at) from credentials where for_stream=false group by check_out_ip;

select check_out_ip, check_out_at from credentials where for_stream=false order by check_out_at limit 1;

show profiles;
