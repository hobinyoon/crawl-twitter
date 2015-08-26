fn_tweet = "../data/hashtag-wimbledon"
fn_output = "../data/wimbledon-by-time"
bucket_size = 2


def ParseOpts(argv):
	if len(argv) >= 2:
		global bucket_size
		bucket_size = int(argv[1])
