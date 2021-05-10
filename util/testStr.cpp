//借助strtok实现split
 #include <string.h>
 #include <stdio.h>

 int main()
 {
        char s[] = "Golden Global   \n   View,disk * desk";
        const char *d = "\n";
         char *p;
         p = strtok(s,d);
         while(p)
         {
                printf("%s\n",p);
                 p=strtok(NULL,d);
         }
 
         return 0;
}