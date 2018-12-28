#!/usr/bin/python3

import pymysql
import pandas as pd
import argparse
import camelot
import sqlalchemy

parser = argparse.ArgumentParser(description='PDF Page to SQL')
parser.add_argument('-i', '--input', help="PDF file name", required=True)
parser.add_argument('-p', '--page', help="Enter page num", required=True)
# parser.add_argument('-f', '--front', help="Front value", nargs='?', const=2, type=int, default=1)
parser.add_argument('-f', '--front', help="Front value", required=True)
args = parser.parse_args()

# show Values
print("input file name {}".format(args.input))
print("Page num is {}".format(args.page))
print("Front Page val is {}".format(args.front))

# Workflow part 1
# After the file is generated
# for the first Page

# BLOCK PDF
# Conversion to csv file
tables = camelot.read_pdf(args.input, pages=args.front)
tables[0].to_csv('foo.csv')
tables.export('foo_allTables.csv', f='csv')
# FILE TO BE FORMED IN CURR DIR -- # foo_allTables - page - 2 - table - 1.csv

# SUPPORT FOR THE JSON AND EXCEL FILES FOR THE FUTURE USAGE

# conversion to json file
# tables[0].to_json('foo.json')
# tables.export('foo_json_allTables_.json', f='json')

# conversion to exel file
# tables[0].to_excel('foo.excel')
# tables.export('foo_excel_allTables_.xls', f='excel')

# final data frame used for various purposes
data_cin = tables[0].df  # will be used for the sql table
# print(data) ## FOR THE TESTING PURPOSES ONLY

# Segement2
# Will get cin num from here
df = pd.read_csv('foo_allTables-page-{}-table-1.csv'.format(args.front))
df_cin = df[df['Unnamed: 0'].str.contains("Corporate identity number")]
# print(df_cin)  # contains the cin row

# Taking out cin number from the 2nd column
print(df_cin.shape[1])
print(range(df_cin.shape[1]))
df_cin.columns = range(df_cin.shape[1])
print(df_cin)
# df_cin_val = df_cin['1'].values[0]
# df_cin_val = df.at[1, 1]
# print(df_cin_val)

df_cin_val = df_cin.loc[0:2, 1]
cin_num = df_cin_val.iloc[0]  # select 0-2 rows
# temp = temp2.iloc[0:1]
print(cin_num)

# Segement3
# FINAL PARSING FOR THE TABLE
tables = camelot.read_pdf(args.input, pages=args.page)
tables[0].to_csv('foo.csv')
tables.export('foo_allTables.csv', f='csv')

data_dpTable = tables[0].df  # will be used for the sql table
# print(data_dpTable)  # FOR THE TESTING PURPOSES ONLY


# Segement4
df = pd.read_csv('foo_allTables-page-{}-table-1.csv'.format(args.page))
# DATA POINTS TO BE PARSED FROM HERE
df_prop = df[df['Unnamed: 0'].str.contains("Property, plant and equipment")]  # 1st rows of data point
df_capital = df[df['Unnamed: 0'].str.contains("Capital work-in-progress")]
df_assets = df[df['Unnamed: 0'].str.contains("Other intangible assets")]
df_dev = df[df['Unnamed: 0'].str.contains("Intangible assets under development")]
df_plants = df[df['Unnamed: 0'].str.contains("Biological assets other than bearer plants")]
df_invest = df[df['Unnamed: 0'].str.contains("Non-current investments")]
df_curr = df[df['Unnamed: 0'].str.contains("Loans, non-current")]
df_finan = df[df['Unnamed: 0'].str.contains("Total non-current financial assets")]
df_ncasets = df[df['Unnamed: 0'].str.contains("Other non-current assets")]

# # TEST THE DATAPOINTS
# print(df_prop)  # contains the cin row
# print(df_capital)
# print(df_assets)
# print(df_dev)
# print(df_plants)
# print(df_invest)
# print(df_curr)
# print(df_finan)
# print(df_ncasets)

# TRANSPOSE DATAPOINTS
t_df_prop = pd.melt(df_prop, id_vars=['Unnamed: 0'],
                    var_name='Dates', value_name="value")
t_df_capital = pd.melt(df_capital, id_vars=['Unnamed: 0'],
                       var_name='Dates', value_name="value")
t_df_assets = pd.melt(df_assets, id_vars=['Unnamed: 0'],
                      var_name='Dates', value_name="value")
t_df_dev = pd.melt(df_dev, id_vars=['Unnamed: 0'],
                   var_name='Dates', value_name="value")
t_df_plants = pd.melt(df_plants, id_vars=['Unnamed: 0'],
                      var_name='Dates', value_name="value")
t_df_invest = pd.melt(df_invest, id_vars=['Unnamed: 0'],
                      var_name='Dates', value_name="value")
t_df_curr = pd.melt(df_curr, id_vars=['Unnamed: 0'],
                    var_name='Dates', value_name="value")
t_df_finan = pd.melt(df_finan, id_vars=['Unnamed: 0'],
                     var_name='Dates', value_name="value")
t_df_ncasets = pd.melt(df_ncasets, id_vars=['Unnamed: 0'],
                       var_name='Dates', value_name="value")

# Testing the Transpose of datapoints
# print(t_df_prop)
# print(t_df_capital)
# print(t_df_assets)
# print(t_df_dev)
# print(t_df_plants)
# print(t_df_invest)
# print(t_df_curr)
# print(t_df_finan)
# print(t_df_ncasets)

print()

# Segment5
# Now final Contanetaion for the table
# df_final = pd.concat(t_df_prop, t_df_capital, t_df_assets, t_df_dev, t_df_plants, t_df_invest, t_df_curr, t_df_finan, t_df_ncasets, axis=1, sort=False)
result = t_df_prop.append([t_df_capital, t_df_assets, t_df_dev, t_df_plants, t_df_invest, t_df_curr, t_df_finan, t_df_ncasets])
# print(result)


result['cin_num'] = cin_num
print(result)

# Segment6
# post data to phpmyadmin
engine = sqlalchemy.create_engine('mysql+pymysql://root:root@localhost/data_2')
result.to_sql(
    name='newTestingTable4',
    con=engine,
    index=True,
    if_exists='append'
)


# References Used
# https://stackoverflow.com/questions/36688022/removing-header-column-from-pandas-dataframe
# https://www.w3resource.com/python-exercises/pandas/python-pandas-data-frame-exercise-20.php
# https://pandas.pydata.org/pandas-docs/stable/merging.html
# http://pandas.pydata.org/pandas-docs/stable/10min.html
# https://www.youtube.com/watch?v=skGwKh1dAdk
# https://stackoverflow.com/questions/20107570/removing-index-column-in-pandas
