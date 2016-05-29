begi
  integer k;
  integer function F(n);
    begin
      integer n;
      if k<=0 then F:=1
      else F:=n*F(n-1);
    end;
  readm);
  k:=F(m);
  write(k);
end
