#!/usr/bin/env python
# coding: utf-8
import numpy as np
import os

filename_1 = input("Filename 1:\n")
filename_2 = input("Filename 2:\n")

dirname = os.path.dirname(__file__)
filename_1 = os.path.join(dirname, filename_1)
file_1 = open(filename_1)
content_1 = file_1.read()
lines_1 = content_1.splitlines()

filename_2 = os.path.join(dirname, filename_2)
file_2 = open(filename_2)
content_2 = file_2.read()
lines_2 = content_2.splitlines()

table_start_1 = ["---" in x for x in lines_1].index(True) + 1
table_end_1 = ["---" in x for x in lines_1[table_start_1:]].index(True)

table_start_2 = ["---" in x for x in lines_2].index(True) + 1
table_end_2 = ["---" in x for x in lines_2[table_start_2:]].index(True)

table_1 = [line.split() for line in lines_1[table_start_1:table_end_1]]

table_2 = [line.split() for line in lines_2[table_start_2:table_end_2]]

#mods_1 = np.array([(float(line[2])**2 + float(line[3])**2 + float(line[4])**2)**0.5 for line in table_1])
#mods_2 = np.array([(float(line[2])**2 + float(line[3])**2 + float(line[4])**2)**0.5 for line in table_2])

mods = np.array([((float(table_1[i][2])-float(table_2[i][2]))**2 + (float(table_1[i][3])-float(table_2[i][3]))**2 + (float(table_1[i][4])-float(table_2[i][4]))**2)**0.5 for i in range(len(table_2))])

#avg_1, avg_2 = np.mean(mods_1), np.mean(mods_2)
avg = np.mean(mods)
#mse = np.mean((mods_1-mods_2)**2)
mae = np.max(mods)

#print(f'avg {filename_1} = {avg_1:.3e} \navg {filename_2} = {avg_2:.3e}')
print(f'MSE={avg:.3e}, LDE={mae:.3e}')





