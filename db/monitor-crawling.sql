-- system clear;

use twitter4;

set profiling=1;

-- select status, count(*) from users where status IN ('U', 'UC', 'UP') group by status order by status limit 5;
select status, count(*) from users group by status order by status;

-- select count(*), count(distinct uid), count(distinct youtube_video_id) from tweets;
select count(*) as num_youtube_reqs from tweets;

-- select * from meta;

select checked_out_ip, max(checked_out_at), min(checked_out_at) from credentials group by checked_out_ip;

select checked_out_ip, checked_out_at from credentials order by checked_out_at limit 1;

show profiles;
