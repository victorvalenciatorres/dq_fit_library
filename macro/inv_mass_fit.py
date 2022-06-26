from ast import parse
from queue import PriorityQueue
from time import process_time_ns
from tkinter import Canvas
from turtle import color, right
import matplotlib.pyplot as plt
import array as arr
import numpy as np
import yaml
import sys
import argparse
import os
from os import path
sys.path.append('../')
from DQFitter import DQFitter

def main():
    parser = argparse.ArgumentParser(description='Arguments to pass')
    parser.add_argument('cfgFileName', metavar='text', default='config.yml', help='config file name')
    parser.add_argument("--run", help="run the multi trial", action="store_true")
    args = parser.parse_args()

    print('Loading task configuration: ...', end='\r')
    with open(args.cfgFileName, 'r') as ymlCfgFile:
        inputCfg = yaml.load(ymlCfgFile, yaml.FullLoader)
    print('Loading task configuration: Done!')

    #../tutorial/tree.root - data
    #../tutorial/hist.root - histMass
    
    if args.run:
        dqFitter = DQFitter(inputCfg["input"]["input_file_name"], inputCfg["input"]["input_name"])
        dqFitter.SetFitConfig(inputCfg["input"]["pdf_dictionary"])
        dqFitter.MultiTrial()

main()