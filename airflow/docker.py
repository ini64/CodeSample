from datetime import datetime, timedelta
from airflow import DAG
from airflow.operators.bash_operator import BashOperator
from airflow.operators.docker_operator import DockerOperator
from airflow.operators.python_operator import BranchPythonOperator
from airflow.operators.dummy_operator import DummyOperator
from docker.types import Mount

default_args = {
'owner'                 : 'airflow',
'description'           : 'Use of the DockerOperator',
'depend_on_past'        : False,
'start_date'            : datetime(2021, 5, 1),
'email_on_failure'      : False,
'email_on_retry'        : False,
'retries'               : 1,
'retry_delay'           : timedelta(minutes=5)
}


with DAG('himari2', default_args=default_args, schedule_interval="0 */4 * * *", catchup=False) as dag:
    start_dag = DummyOperator(
        task_id='start_dag'
        )

    end_dag = DummyOperator(
        task_id='end_dag'
        )

    t1 = BashOperator(
        task_id='print_current_date',
        bash_command='date'
        )

    t2 = DockerOperator(
        task_id='himari_collector',
        image='centos:latest',
        api_version='auto',
        auto_remove=True,
        docker_url="unix://var/run/docker.sock",
        network_mode="host",
        environment={'ENVIRONMENT':'linux'},
        mount_tmp_dir=False,
        mounts=[
            Mount(source='/home/mx/docker/airflow/var/pos',
              target='/var/pos',
              type='bind'),
            Mount(source='/home/mx/docker/airflow/var/data',
              target='/var/data',
              type='bind'),
        ],
        )

    t4 = BashOperator(
        task_id='print_hello',
        bash_command='echo "hello world"'
        )

    start_dag >> t1

    t1 >> t2 >> t4

    t4 >> end_dag