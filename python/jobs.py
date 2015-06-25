''' Write out a list of jobs that can be used by other scripts, for example, for batch submission.
Mainly for use by the miind script.'''

import os
import errno
import directories

def create_dir(name):
    '''Creates a directory relative to the MIIND_ROOT/build/jobs.'''
    sep = os.path.sep
    directories.initialize_global_variables()
    miind_root = directories.miind_root()
    abspath = os.path.join(miind_root,'build', 'jobs', name)
    return abspath
    
    

def write_out_job(job_name):
    ''' Just a single job needs to be added.'''
    dirname=job_name[:-4]
    if job_name[-4:] != '.xml':
        raise NameError

    abspath=create_dir(dirname)
    try:
        os.makedirs(abspath)
    except OSError as exception:
        if exception.errno != errno.EEXIST:
            raise

    joblistname = os.path.join(abspath,'joblist')
    with open(joblistname,'w') as f:
        path = os.path.join(directories.miind_root(), 'build' ,'apps', job_name)
        f.write(path + '\n')
    
    return

def write_out_jobs(base_name, job_names):
    ''' A directory with job names will be added.'''
    abspath=create_dir(base_name)

    try:
        os.makedirs(abspath)
    except OSError as exception:
        if exception.errno != errno.EEXIST:
            raise
    joblistname = os.path.join(abspath,'joblist')
    with open(joblistname,'w') as f:
        for name in job_names:
            name = directories.check_and_strip_name(name)
            path = os.path.join(directories.miind_root(), 'build' ,'apps', base_name, name, name)
            f.write(path + '\n')

    return abspath