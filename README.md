# como como object meta oriented

```
func isodd(num) {
	i = 0;
	odd = 0;
	while((i == num) == 0) {
		if(odd == 0) {
			odd = 1;
		} else {
			odd = 0;
		}
		i = i + 1;
	}
	return odd;
}

func print_even(num)
{
	while(num) {
		temp = isodd(num);
		if(temp == 0) {
			print(num);
		} else {
			print("odd");
		}
		num = num - 1;
	}
}

print_even(4);
```
