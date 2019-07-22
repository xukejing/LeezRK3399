__kernel void test(__global int *a, __global int *b)
{
    int id = get_global_id(0);
    b[id] = a[id] ;
    int ii,jj,tmp;
    for(ii=0;ii<19;ii++)
	{
		for(jj=0;jj<(19-ii);jj++)
		{
			if(b[jj]<b[jj+1])
			{
				tmp=b[jj];
				b[jj]=b[jj+1];
				b[jj+1]=tmp;
			}
		}
	}
}
