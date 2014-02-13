-- Q1.1
SELECT sum(lo.lo_extendedprice * lo.lo_discount) AS revenue
FROM   ssb_lineorder lo
       JOIN ssb_date d
         ON lo.lo_orderdate = d.d_datekey
WHERE  d.d_year = 1993
       AND lo.lo_discount BETWEEN 1 AND 3
       AND lo.lo_quantity < 25;
       
-- Q1.2
SELECT sum(lo.lo_extendedprice * lo.lo_discount) AS revenue
FROM   ssb_lineorder lo
       JOIN ssb_date d
         ON lo.lo_orderdate = d.d_datekey
WHERE  d.d_yearmonth = 'Jan1994'
       AND lo.lo_discount BETWEEN 4 AND 6
       AND lo.lo_quantity BETWEEN 26 AND 35;
       
-- Q1.3
SELECT sum(lo.lo_extendedprice * lo.lo_discount) AS revenue
FROM   ssb_lineorder lo
       JOIN ssb_date d
         ON lo.lo_orderdate = d.d_datekey
WHERE  d.d_weeknuminyear = 6
       AND d.d_year = 1994
       AND lo.lo_discount BETWEEN 5 AND 7
       AND lo.lo_quantity BETWEEN 26 AND 35;
       
-- Q2.1
SELECT sum(lo.lo_revenue) lo_revenue,
       d.d_year,
       p.p_brand
FROM   ssb_lineorder lo
       JOIN ssb_date d
         ON lo.lo_orderdate = d.d_datekey
       JOIN ssb_part p
         ON lo.lo_partkey = p.p_partkey
       JOIN ssb_supplier s
         ON lo.lo_suppkey = s.s_suppkey
WHERE  p.p_category = 'MFGR#12'
       AND s.s_region = 'AMERICA'
GROUP  BY d.d_year,
          p.p_brand
ORDER  BY d_year,
          p_brand;
          
-- Q2.2
SELECT sum(lo.lo_revenue) lo_revenue,
       d.d_year,
       p.p_brand
FROM   ssb_lineorder lo
       JOIN ssb_date d
         ON lo.lo_orderdate = d.d_datekey
       JOIN ssb_part p
         ON lo.lo_partkey = p.p_partkey
       JOIN ssb_supplier s
         ON lo.lo_suppkey = s.s_suppkey
WHERE  p.p_brand BETWEEN 'MFGR#2221' AND 'MFGR#2228'
       AND s.s_region = 'ASIA'
GROUP  BY d.d_year,
          p.p_brand
ORDER  BY d_year,
          p_brand;
          
-- Q2.3
SELECT sum(lo.lo_revenue) lo_revenue,
       d.d_year,
       p.p_brand
FROM   ssb_lineorder lo
       JOIN ssb_date d
         ON lo.lo_orderdate = d.d_datekey
       JOIN ssb_part p
         ON lo.lo_partkey = p.p_partkey
       JOIN ssb_supplier s
         ON lo.lo_suppkey = s.s_suppkey
WHERE  p.p_brand = 'MFGR#2238'
       AND s.s_region = 'EUROPE'
GROUP  BY d.d_year,
          p.p_brand
ORDER  BY d_year,
          p_brand;
          
-- Q3.1
SELECT c.c_nation,
       s.s_nation,
       d.d_year,
       sum(lo.lo_revenue) AS revenue
FROM   ssb_lineorder lo
       JOIN ssb_customer c
         ON lo.lo_custkey = c.c_custkey
       JOIN ssb_supplier s
         ON lo.lo_suppkey = s.s_suppkey
       JOIN ssb_date d
         ON lo.lo_orderdate = d.d_datekey
WHERE  c.c_region = 'ASIA'
       AND s.s_region = 'ASIA'
       AND d.d_year >= 1992
       AND d.d_year <= 1997
GROUP  BY c.c_nation,
          s.s_nation,
          d.d_year
ORDER  BY d_year ASC,
          revenue DESC;

-- Q3.2
SELECT c.c_city,
       s.s_city,
       d.d_year,
       sum(lo.lo_revenue) AS revenue
FROM   ssb_lineorder lo
       JOIN ssb_customer c
         ON lo.lo_custkey = c.c_custkey
       JOIN ssb_supplier s
         ON lo.lo_suppkey = s.s_suppkey
       JOIN ssb_date d
         ON lo.lo_orderdate = d.d_datekey
WHERE  c.c_nation = 'CHINA'
       AND s.s_nation = 'CHINA'
       AND d.d_year >= 1992
       AND d.d_year <= 1997
GROUP  BY c.c_city,
          s.s_city,
          d.d_year
ORDER  BY d_year ASC,
          revenue DESC
          
-- Q3.3
SELECT c.c_city,
       s.s_city,
       d.d_year,
       sum(lo.lo_revenue) AS revenue
FROM   ssb_lineorder lo
       JOIN ssb_customer c
         ON lo.lo_custkey = c.c_custkey
       JOIN ssb_supplier s
         ON lo.lo_suppkey = s.s_suppkey
       JOIN ssb_date d
         ON lo.lo_orderdate = d.d_datekey
WHERE  ( c.c_city = 'UNITED KI0'
          OR c.c_city = 'UNITED KI6' )
       AND ( s.s_city = 'UNITED KI0'
              OR s.s_city = 'UNITED KI6' )
       AND d.d_year >= 1992
       AND d.d_year <= 1997
GROUP  BY c.c_city,
          s.s_city,
          d.d_year
ORDER  BY d_year ASC,
          revenue DESC
          
-- Q3.4
SELECT c.c_city,
       s.s_city,
       d.d_year,
       sum(lo.lo_revenue) AS revenue
FROM   ssb_lineorder lo
       JOIN ssb_customer c
         ON lo.lo_custkey = c.c_custkey
       JOIN ssb_supplier s
         ON lo.lo_suppkey = s.s_suppkey
       JOIN ssb_date d
         ON lo.lo_orderdate = d.d_datekey
WHERE  ( c.c_city = 'UNITED KI0'
          OR c.c_city = 'UNITED KI6' )
       AND ( s.s_city = 'UNITED KI0'
              OR s.s_city = 'UNITED KI6' )
       AND d.d_yearmonth = 'May1992'  
GROUP  BY c.c_city,
          s.s_city,
          d.d_year
ORDER  BY d_year ASC,
          revenue DESC
          
-- Q4.1
SELECT d.d_year,
       c.c_nation,
       sum(lo.lo_revenue - lo.lo_supplycost) AS profit
FROM   ssb_lineorder lo
       JOIN ssb_date d
         ON lo.lo_orderdate = d.d_datekey
       JOIN ssb_customer c
         ON lo.lo_custkey = c.c_custkey
       JOIN ssb_supplier s
         ON lo.lo_suppkey = s.s_suppkey
       JOIN ssb_part p
         ON lo.lo_partkey = p.p_partkey
WHERE  c.c_region = 'AMERICA'
       AND s.s_region = 'AMERICA'
       AND ( p.p_mfgr = 'MFGR#1'
              OR p.p_mfgr = 'MFGR#2' )
GROUP  BY d.d_year,
          c.c_nation
ORDER  BY d_year,
          c_nation
          
-- Q4.2
SELECT d.d_year,
       s.s_nation,
       p.p_category,
       sum(lo.lo_revenue - lo.lo_supplycost) AS profit
FROM   ssb_lineorder lo
       JOIN ssb_date d
         ON lo.lo_orderdate = d.d_datekey
       JOIN ssb_customer c
         ON lo.lo_custkey = c.c_custkey
       JOIN ssb_supplier s
         ON lo.lo_suppkey = s.s_suppkey
       JOIN ssb_part p
         ON lo.lo_partkey = p.p_partkey
WHERE  c.c_region = 'AMERICA'
       AND s.s_region = 'AMERICA'
       AND ( d.d_year = 1997
              OR d.d_year = 1998 )
       AND ( p.p_mfgr = 'MFGR#1'
              OR p.p_mfgr = 'MFGR#2' )
GROUP  BY d.d_year,
          s.s_nation,
          p.p_category
ORDER  BY d_year,
          s_nation,
          p_category
          
-- Q4.3
SELECT d.d_year,
       s.s_city,
       p.p_brand,
       sum(lo.lo_revenue - lo.lo_supplycost) AS profit
FROM   ssb_lineorder lo
       JOIN ssb_date d
         ON lo.lo_orderdate = d.d_datekey
       JOIN ssb_customer c
         ON lo.lo_custkey = c.c_custkey
       JOIN ssb_supplier s
         ON lo.lo_suppkey = s.s_suppkey
       JOIN ssb_part p
         ON lo.lo_partkey = p.p_partkey
WHERE  s.s_nation = 'UNITED STATES'
       AND ( d.d_year = 1997
              OR d.d_year = 1998 )
       AND p.p_category = 'MFGR#14'
GROUP  BY d.d_year,
          s.s_city,
          p.p_brand
ORDER  BY d_year,
          s_city,
          p_brand