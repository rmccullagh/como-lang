func get_space() {
	return " ";
}

func make_name(first, last)
{
	return first + get_space() + last;
}

func init()
{
	for(i = 0; i < 5; i++) {
		print(make_name("Ryan (" + i + ")", "McCullagh"));
	}
}

init();
