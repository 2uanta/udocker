# Run udocker on Gullimin HPC cluster
## Run in interactive mode
* Request a node via qsub
```
qsub -I -X -l nodes=1:ppn=16:mics=2 -l walltime=1:00:00
```
* Once connected to a worker node, run udocker:
```
udocker run ubuntu bash
```
Replace `ubuntu bash` with your container name and the initial command to be executed.
## Run a batch job
* submit udocker command to normal qusb
```
echo "udocker run r-base R \
    CMD BATCH /home/user1/power-stats-reports/batch-demo.R power-stats-reports/batch-demo.Rout" | \
    qsub -l nodes=1:ppn=16:mics=2
```
Enter the command and parameters to be run as a background task after `udocker run`.
