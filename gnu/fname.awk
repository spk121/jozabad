# Check to see if the name of the file appears in the second name
# of the file
BEGIN {
    n=split(fname,a,"/"); 
    bname=a[n];
}
{
    if(NR==2 && index($0,bname) == 0)
        print fname;
}
