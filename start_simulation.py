import os
import sys

if len(sys.argv) != 2: 
    repetitions = 1
else: 
    repetitions = int(sys.argv[1])

# Set parameters values
size = '1000'
cycles = '30'
cycle_length = '1000'
drop = '0'
max_trans_per_block = '50'
reward = '10'
trans_prob = '0.15'
prob_cpu = '0.10'
prob_gpu = '0.30'
prob_fpga = '0.30'
prob_asic = '0.30'
prob_miner = ['0.30']
prob_sminer = [ '0.10', '0.30', '0.50', '0.70','0.90', '1.00'] 
prob_2miners = ['0.05']
delay = ['0', '10', '50', '90']


# Read in the file
with open('tinycoin_config_for_script.txt', 'r') as file :
  config = file.read()

# Substitute in the configuration file the parameters that have a single value
config = config.replace('SIZE', size)
config = config.replace('CYCLES', cycles)
config = config.replace('CYCLE_LENGTH', cycle_length)
config = config.replace('DROP', drop)
config = config.replace('MAX_TRANS_PER_BLOCK', max_trans_per_block)
config = config.replace('REWARD', reward)
config = config.replace('TRANS_PROB', trans_prob)
config = config.replace('PROB_CPU', prob_cpu)
config = config.replace('PROB_GPU', prob_gpu)
config = config.replace('PROB_FPGA', prob_fpga)
config = config.replace('PROB_ASIC', prob_asic)
config = config.replace('PROB_MINER', prob_miner[0])
config = config.replace('PROB_2MINERS', prob_2miners[0])
configd = config                                               # Use configd to configure simulations with various values of delay
configd = configd.replace('ONLYLATENCY', 'true')
configd = configd.replace('PROB_SMINER', '0.10')
config = config.replace('DELAY', '0')
config = config.replace('ONLYLATENCY', 'false')


repetitions = range(1,repetitions+1)
for count in repetitions:
# Replace the parameters in the configuration file and run the simulation   
    config_overwrite = config 
    config_overwrite = config_overwrite.replace('REPETITION', str(count))
    for p in prob_sminer:
        config_overwrite2 = config_overwrite  
        config_overwrite2 = config_overwrite2.replace('PROB_SMINER', p)
        # Write the file out again
        with open('tinycoin_config_overwritten.txt', 'w+') as file:
            file.write(config_overwrite2)
        os.system('java -cp lib/jep-2.3.0.jar:lib/djep-1.0.0.jar:lib/peersim-1.0.5.jar:lib/tinycoin.jar peersim.Simulator tinycoin_config_overwritten.txt ')
   
    # Run the simulation also with different values of delay        
    for d in delay:    
        config_overwrite = configd    
        config_overwrite = config_overwrite.replace('REPETITION', str(count))     
        config_overwrite = config_overwrite.replace('DELAY', d)        
        with open('tinycoin_config_overwritten.txt', 'w+') as file:
            file.write(config_overwrite)
        os.system('java -cp lib/jep-2.3.0.jar:lib/djep-1.0.0.jar:lib/peersim-1.0.5.jar:lib/tinycoin.jar peersim.Simulator tinycoin_config_overwritten.txt ')
        
        
# Make the averages of the various statistics
for p in prob_sminer:
    os.system('python build_avg_statistics.py ' + p)
    
for d in delay:
    os.system('python build_avg_statistics.py d' + d)
    
# Plot the averaged statistics    
os.system('python plot_avg_statistics.py')
